#include "stdafx.h"
#include "common.hpp"
#include "logging.hpp"
#include "game_funcs.hpp"
#include "mg1_linkvarbuf.hpp"
#include "mg1_text_strings.hpp"
#include "mg2_text_strings.hpp"
#include "mg2_ending_cast.hpp"
#include "mg2_boss_name_sprites.hpp"
#include "mg2_sprite_patcher.hpp"
#include "mg_text_encoding.hpp"
#include "mg_text_overrides.hpp"
#include "mg_vector_font.hpp"
#include "translation_diffs.hpp"

namespace
{
    using TextKey = std::pair<uint32_t, int32_t>;
    using TextTable = std::map<TextKey, std::string>;
    std::map<int, TextTable> s_mMG1Text;
    std::map<int, TextTable> s_mMG2Text;
    SafetyHookInline s_hMG1ChunkData;
    SafetyHookInline s_hMG1ChunkSize;
    SafetyHookInline s_hMG2Text;

    struct TextChunk
    {
        const uint8_t* pSource = nullptr;
        int nLanguage = 0;
        std::vector<uint8_t> vOriginal;
        std::vector<uint8_t> vReplacement;
    };
    std::deque<TextChunk> s_vMG1Chunks;
    TextChunk* s_pCurrentChunk = nullptr;

    int Language(const char* pszCode)
    {
        constexpr std::pair<std::string_view, MGLanguage> astLanguages[] =
        {
            { "en", MGLanguage::English }, { "fr", MGLanguage::French },
            { "gr", MGLanguage::German }, { "it", MGLanguage::Italian },
            { "sp", MGLanguage::Spanish }, { "jp", MGLanguage::Japanese },
        };
        for (const auto& [svCode, eLanguage] : astLanguages)
        {
            if (pszCode && svCode == pszCode)
            {
                return static_cast<int>(eLanguage);
            }
        }
        return 0;
    }

    bool Encode(const char* pszText, int nLanguage, bool bMG2, std::string& sEncoded)
    {
        if (!pszText)
        {
            return false;
        }
        const int nLength = static_cast<int>(strlen(pszText));
        if (nLength == 0)
        {
            return true;
        }
        const int nWideLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, pszText, nLength, nullptr, 0);
        if (nWideLength == 0)
        {
            return false;
        }
        std::wstring sWide(nWideLength, L'\0');
        MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, pszText, nLength, sWide.data(), nWideLength);
        if (nLanguage == static_cast<int>(MGLanguage::Japanese))
        {
            for (size_t nIndex = 0; nIndex < sWide.size(); ++nIndex)
            {
                if (bMG2)
                {
                    const auto itCharacter = std::ranges::find(astMG2JapaneseCharacters, sWide[nIndex], &MGTextCharacter::nUnicode);
                    if (itCharacter != std::end(astMG2JapaneseCharacters))
                    {
                        sEncoded.push_back(static_cast<char>(itCharacter->nByte));
                        continue;
                    }
                }
                const int nCharacters = sWide[nIndex] >= 0xD800 && sWide[nIndex] <= 0xDBFF && nIndex + 1 < sWide.size() ? 2 : 1;
                char achEncoded[4] {};
                BOOL bDefault = FALSE;
                const int nBytes = WideCharToMultiByte(932, WC_NO_BEST_FIT_CHARS, sWide.data() + nIndex, nCharacters, achEncoded, sizeof(achEncoded), nullptr, &bDefault);
                if (nBytes <= 0 || bDefault)
                {
                    const uint16_t nCode = nCharacters == 1 && MG_VectorFont::bEnabled ? MGTextExtended::Allocate(sWide[nIndex], true) : 0;
                    if (!nCode)
                    {
                        return false;
                    }
                    sEncoded.push_back(static_cast<char>(nCode >> 8));
                    sEncoded.push_back(static_cast<char>(nCode & 0xFF));
                    continue;
                }
                sEncoded.append(achEncoded, nBytes);
                nIndex += nCharacters - 1;
            }
            return true;
        }
        const std::span<const MGTextCharacter> stCharacters = bMG2 ? std::span<const MGTextCharacter>(astMG2WesternCharacters) : std::span<const MGTextCharacter>(astMG1WesternCharacters);
        for (wchar_t nCharacter : sWide)
        {
            if (nCharacter < 0x80)
            {
                sEncoded.push_back(static_cast<char>(nCharacter));
                continue;
            }
            const auto itCharacter = std::ranges::find(stCharacters, nCharacter, &MGTextCharacter::nUnicode);
            if (itCharacter == stCharacters.end())
            {
                const uint16_t nCode = MG_VectorFont::bEnabled ? MGTextExtended::Allocate(nCharacter, false) : 0;
                if (!nCode)
                {
                    return false;
                }
                sEncoded.push_back(static_cast<char>(nCode >> 8));
                sEncoded.push_back(static_cast<char>(nCode & 0xFF));
                continue;
            }
            sEncoded.push_back(static_cast<char>(itCharacter->nByte));
        }
        return true;
    }

    void Load(std::span<const MGTextOverride> vEntries, bool bMG2, std::map<int, TextTable>& mTables)
    {
        for (const MGTextOverride& stEntry : vEntries)
        {
            const int nLanguage = Language(stEntry.pszLanguage);
            std::string sText;
            if (!nLanguage || stEntry.nIndex < 0 || !Encode(stEntry.pszReplacementUtf8, nLanguage, bMG2, sText) || sText.size() > (bMG2 ? 32767u : 2047u))
            {
                spdlog::error("MG{}: Text Overrides : Invalid replacement for block {}, index {}.", bMG2 ? 2 : 1, stEntry.nBlock, stEntry.nIndex);
                continue;
            }
            mTables[nLanguage][{ stEntry.nBlock, stEntry.nIndex }] = std::move(sText);
        }
    }

    struct RowSite
    {
        uint8_t* pInstruction;
        int nRow;
        std::array<uint8_t, 3> aPrefix;
    };

    struct ByteEdit
    {
        uint8_t* pAddress;
        std::vector<uint8_t> vExpected;
        std::vector<uint8_t> vReplacement;
    };

    // Expanded glpyhs use 2 bytes instead of 1 when gm_lang != japanese, which can cause messages with a lot of fancy accents to push past the vanilla text buffer size. increases the buffer size from 48 bytes to 96 bytes
    bool WidenMG2MessageRows()
    {
        constexpr int nRows = 10;
        constexpr int nOldStride = 48;
        constexpr int nNewStride = 96;
        constexpr int nStoreLength = 7;
        uint8_t* pMessage = reinterpret_cast<uint8_t*>(MG2_Gamefuncs::pfnDrawMessageText);
        uint8_t* pStory = reinterpret_cast<uint8_t*>(MG2_Gamefuncs::pfnDrawStoryText);
        uint8_t* pFrameBuffers = reinterpret_cast<uint8_t*>(MG2_Gamefuncs::pfnInitFrameBuffers);
        if (!pMessage || !pStory || !pFrameBuffers)
        {
            return false;
        }
        std::vector<RowSite> vSites;
        const auto AddClear = [&](uint8_t* pFirst, uint8_t nRegister)
        {
            for (int nIndex = 0; nIndex < nRows; ++nIndex)
            {
                vSites.push_back({ pFirst + nIndex * nStoreLength, nRows - 1 - nIndex, { 0x40, 0x88, nRegister } });
            }
        };
        //DrawMessageText+8E
        AddClear(pMessage + 0x8E, 0x2D);
        //DrawMessageText+6AE
        AddClear(pMessage + 0x6AE, 0x2D);
        //DrawStoryText+52
        AddClear(pStory + 0x52, 0x35);
        //DrawMessageText+E5
        vSites.push_back({ pMessage + 0xE5, -1, { 0x4C, 0x8D, 0x2D } });
        //DrawStoryText+A8
        vSites.push_back({ pStory + 0xA8, -1, { 0x48, 0x8D, 0x2D } });

        std::vector<ByteEdit> vEdits;
        const auto AddShift = [&](uint8_t* pInstruction, uint8_t nRegister)
        {
            vEdits.push_back({ pInstruction, { 0x48, 0xC1, nRegister, 0x04 }, { 0x48, 0xC1, nRegister, 0x05 } });
        };
        const auto AddSubtract = [&](uint8_t* pInstruction, uint8_t nRegister)
        {
            vEdits.push_back({ pInstruction, { 0x48, 0x83, nRegister, 0x30 }, { 0x48, 0x83, nRegister, 0x60 } });
        };
        //DrawMessageText+1B7
        AddShift(pMessage + 0x1B7, 0xE1);
        //DrawMessageText+26A
        AddShift(pMessage + 0x26A, 0xE3);
        //DrawMessageText+56C
        AddShift(pMessage + 0x56C, 0xE1);
        //DrawMessageText+5A1
        AddShift(pMessage + 0x5A1, 0xE1);
        //DrawStoryText+150
        AddShift(pStory + 0x150, 0xE1);
        //DrawStoryText+175
        AddShift(pStory + 0x175, 0xE1);
        //DrawStoryText+1E4
        AddShift(pStory + 0x1E4, 0xE7);
        //DrawMessageText+29A
        AddSubtract(pMessage + 0x29A, 0xEB);
        //DrawStoryText+237
        AddSubtract(pStory + 0x237, 0xEF);
        //InitFrameBuffers+36 : text item size 0x44 becomes 0x7C
        vEdits.push_back({ pFrameBuffers + 0x36, { 0x41, 0x8D, 0x50, 0x44 }, { 0x41, 0x8D, 0x50, 0x7C } });
        //DrawMessageText+30C : row * 6 * 8 becomes row * 12 * 8
        vEdits.push_back({ pMessage + 0x30C, { 0x4C, 0x8D, 0x04, 0x40 }, { 0x4C, 0x6B, 0xC0, 0x0C } });
        vEdits.push_back({ pMessage + 0x310, { 0x4D, 0x03, 0xC0 }, { 0x0F, 0x1F, 0x00 } });

        const uint8_t* pOldRows = nullptr;
        for (const RowSite& stSite : vSites)
        {
            if (!std::equal(stSite.aPrefix.begin(), stSite.aPrefix.end(), stSite.pInstruction))
            {
                return false;
            }
            const uint8_t* pTarget = stSite.pInstruction + nStoreLength + *reinterpret_cast<const int32_t*>(stSite.pInstruction + 3);
            const uint8_t* pBase = pTarget - (stSite.nRow < 0 ? 0 : stSite.nRow * nOldStride);
            if (pOldRows && pBase != pOldRows)
            {
                return false;
            }
            pOldRows = pBase;
        }
        for (const ByteEdit& stEdit : vEdits)
        {
            if (!std::equal(stEdit.vExpected.begin(), stEdit.vExpected.end(), stEdit.pAddress))
            {
                return false;
            }
        }
        //InitFrameBuffers+1E
        const uint8_t* pItemPool = pFrameBuffers + 0x1E;
        if (pItemPool[0] != 0x48 || pItemPool[1] != 0x8D || pItemPool[2] != 0x1D || *reinterpret_cast<const uint64_t*>(pItemPool + 7 + *reinterpret_cast<const int32_t*>(pItemPool + 3)) != 0)
        {
            return false;
        }
        uint8_t* pNewRows = nullptr;
        for (uintptr_t nOffset = 0x01000000; nOffset < 0x70000000 && !pNewRows; nOffset += 0x01000000)
        {
            pNewRows = static_cast<uint8_t*>(VirtualAlloc(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(pMessage) + nOffset), nRows * nNewStride, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
        }
        if (!pNewRows)
        {
            return false;
        }
        for (const RowSite& stSite : vSites)
        {
            const uint8_t* pTarget = pNewRows + (stSite.nRow < 0 ? 0 : stSite.nRow * nNewStride);
            Memory::Write<int32_t>(reinterpret_cast<uintptr_t>(stSite.pInstruction + 3), static_cast<int32_t>(pTarget - (stSite.pInstruction + nStoreLength)));
        }
        for (const ByteEdit& stEdit : vEdits)
        {
            for (size_t nIndex = 0; nIndex < stEdit.vReplacement.size(); ++nIndex)
            {
                Memory::Write<uint8_t>(reinterpret_cast<uintptr_t>(stEdit.pAddress + nIndex), stEdit.vReplacement[nIndex]);
            }
        }
        return true;
    }

    uint32_t ReadSize(const uint8_t* pData)
    {
        return (uint32_t(pData[0]) << 24) | (uint32_t(pData[1]) << 16) | (uint32_t(pData[2]) << 8) | pData[3];
    }

    bool RebuildChunk(std::span<const uint8_t> stSource, const TextTable& mText, std::vector<uint8_t>& vOutput)
    {
        size_t nOffset = 0;
        uint32_t nBlock = 0;
        while (nOffset < stSource.size())
        {
            if (stSource.size() - nOffset < 4)
            {
                return false;
            }
            const uint32_t nHeader = ReadSize(stSource.data() + nOffset);
            const size_t nLength = nHeader & 0x3FFFFFFF;
            if (nLength > stSource.size() - nOffset - 4)
            {
                return false;
            }
            const size_t nStart = nOffset + 4;
            const size_t nEnd = nStart + nLength;
            const auto itBlock = mText.lower_bound({ nBlock, 0 });
            if (itBlock == mText.end() || itBlock->first.first != nBlock)
            {
                vOutput.insert(vOutput.end(), stSource.begin() + nOffset, stSource.begin() + nEnd);
            }
            else
            {
                const size_t nHeaderOffset = vOutput.size();
                vOutput.resize(nHeaderOffset + 4);
                int32_t nIndex = 0;
                size_t nRecord = nStart;
                while (nRecord < nEnd)
                {
                    if (nEnd - nRecord < 3)
                    {
                        return false;
                    }
                    const size_t nTextLength = (size_t(stSource[nRecord + 1]) << 8) | stSource[nRecord + 2];
                    if (nTextLength > nEnd - nRecord - 3)
                    {
                        return false;
                    }
                    const auto itText = mText.find({ nBlock, nIndex });
                    if (itText == mText.end())
                    {
                        vOutput.insert(vOutput.end(), stSource.begin() + nRecord, stSource.begin() + nRecord + 3 + nTextLength);
                    }
                    else
                    {
                        const std::string& sText = itText->second;
                        vOutput.push_back(stSource[nRecord]);
                        vOutput.push_back(static_cast<uint8_t>(sText.size() >> 8));
                        vOutput.push_back(static_cast<uint8_t>(sText.size()));
                        vOutput.insert(vOutput.end(), sText.begin(), sText.end());
                    }
                    nRecord += 3 + nTextLength;
                    ++nIndex;
                }
                const uint32_t nNewHeader = (nHeader & 0xC0000000) | static_cast<uint32_t>(vOutput.size() - nHeaderOffset - 4);
                for (int nByte = 0; nByte < 4; ++nByte)
                {
                    vOutput[nHeaderOffset + nByte] = static_cast<uint8_t>(nNewHeader >> (24 - 8 * nByte));
                }
            }
            nOffset = nEnd;
            ++nBlock;
        }
        return true;
    }

    TextChunk* GetTextChunk(const uint8_t* pSource, uint32_t nSize)
    {
        const int nLanguage = Shared_Gamefuncs::pfnGetLanguage();
        const auto itTable = s_mMG1Text.find(nLanguage);
        if (itTable == s_mMG1Text.end() || !pSource || nSize > 16 * 1024 * 1024)
        {
            return nullptr;
        }
        if (s_pCurrentChunk && s_pCurrentChunk->pSource == pSource && s_pCurrentChunk->nLanguage == nLanguage && s_pCurrentChunk->vOriginal.size() == nSize && memcmp(pSource, s_pCurrentChunk->vOriginal.data(), nSize) == 0)
        {
            return s_pCurrentChunk;
        }
        TextChunk stChunk;
        stChunk.pSource = pSource;
        stChunk.nLanguage = nLanguage;
        stChunk.vOriginal.assign(pSource, pSource + nSize);
        if (!RebuildChunk(stChunk.vOriginal, itTable->second, stChunk.vReplacement))
        {
            spdlog::error("MG1: Text Overrides : Invalid text chunk.");
            return nullptr;
        }
        s_vMG1Chunks.push_back(std::move(stChunk)); // Earlier text pointers may still be in use.
        s_pCurrentChunk = &s_vMG1Chunks.back();
        return s_pCurrentChunk;
    }

    const uint8_t* __fastcall GetChunkData(uintptr_t hChunk, int nEntry)
    {
        const uint8_t* pData = s_hMG1ChunkData.call<const uint8_t*>(hChunk, nEntry);
        if (nEntry != 0 || hChunk != *MG1_LinkVarBuf::pTextChunkHandle)
        {
            return pData;
        }
        const uint32_t nSize = s_hMG1ChunkSize.call<uint32_t>(hChunk, nEntry);
        TextChunk* pChunk = GetTextChunk(pData, nSize);
        return pChunk ? pChunk->vReplacement.data() : pData;
    }

    uint32_t __fastcall GetChunkSize(uintptr_t hChunk, int nEntry)
    {
        const uint32_t nSize = s_hMG1ChunkSize.call<uint32_t>(hChunk, nEntry);
        if (nEntry != 0 || hChunk != *MG1_LinkVarBuf::pTextChunkHandle)
        {
            return nSize;
        }
        TextChunk* pChunk = GetTextChunk(s_hMG1ChunkData.call<const uint8_t*>(hChunk, nEntry), nSize);
        return pChunk ? static_cast<uint32_t>(pChunk->vReplacement.size()) : nSize;
    }

    const char* __fastcall GetLocalizedString(uint32_t nBlock, int32_t nIndex, int* pLength)
    {
        const auto itLanguage = s_mMG2Text.find(Shared_Gamefuncs::pfnGetLanguage());
        if (itLanguage != s_mMG2Text.end())
        {
            const auto itText = itLanguage->second.find({ nBlock, nIndex });
            if (itText != itLanguage->second.end())
            {
                if (pLength)
                {
                    *pLength = static_cast<int>(itText->second.size());
                }
                return itText->second.c_str();
            }
        }
        return s_hMG2Text.call<const char*>(nBlock, nIndex, pLength);
    }
}

void MG_TextOverrides::InitializeMG1()
{
    Load(kMG1TypoFixes, false, s_mMG1Text);
    if (bMG1RestoreBossNames)
    {
        Load(kMG1RestoredBossNames, false, s_mMG1Text);
    }
    //if (bMG1RestoreOriginalScript)
    //{
    //    Load(kMG1OriginalScript, false, s_mMG1Text);
    //}
    for (const auto& stEntry : TranslationDiffs::Load("MG1"))
    {
        const MGTextOverride stOverride { stEntry.nBlock, stEntry.nIndex, stEntry.sLanguage.c_str(), stEntry.sText.c_str() };
        Load(std::span<const MGTextOverride>(&stOverride, 1), false, s_mMG1Text);
    }
    if (s_mMG1Text.empty())
    {
        spdlog::info("MG1: Text Overrides : No replacements configured.");
        return;
    }
    if (!MG1_Gamefuncs::pfnGetChunkEntryData || !MG1_Gamefuncs::pfnGetChunkEntrySize || !MG1_LinkVarBuf::pTextChunkHandle || !Shared_Gamefuncs::pfnGetLanguage)
    {
        spdlog::error("MG1: Text Overrides : Required functions or text chunk unavailable.");
        return;
    }
    s_hMG1ChunkData = safetyhook::create_inline(MG1_Gamefuncs::pfnGetChunkEntryData, GetChunkData);
    s_hMG1ChunkSize = safetyhook::create_inline(MG1_Gamefuncs::pfnGetChunkEntrySize, GetChunkSize);
    LOG_HOOK(s_hMG1ChunkData, "MG1: Text Overrides : GetChunkEntryData()")
    LOG_HOOK(s_hMG1ChunkSize, "MG1: Text Overrides : GetChunkEntrySize()")
    if (!s_hMG1ChunkData || !s_hMG1ChunkSize)
    {
        s_hMG1ChunkData.reset();
        s_hMG1ChunkSize.reset();
        return;
    }
    spdlog::info("MG1: Text Overrides : Initialized.");
}

void MG_TextOverrides::InitializeMG2()
{
    MG2_EndingCast::Initialize();
    Load(kMG2TypoFixes, true, s_mMG2Text);
    if (bMG2RestoreBossNames)
    {
        Load(kMG2RestoredBossNames, true, s_mMG2Text);
        bool bRegistered = true;
        for (const auto& stPatch : MG2_BossNameSprites::astPatches)
        {
            bRegistered &= MG2_SpritePatcher::AddPatch(70, stPatch.nOriginalHash, stPatch.aBlob.data(), stPatch.aBlob.size());
        }
        if (bRegistered)
        {
            spdlog::info("MG2: Boss Names : Sprite replacements registered.");
        }
    }
    if (bMG2RestoreOriginalScript)
    {
        Load(kMG2OriginalScript, true, s_mMG2Text);
    }
    for (const auto& stEntry : TranslationDiffs::Load("MG2"))
    {
        const MGTextOverride stOverride { stEntry.nBlock, stEntry.nIndex, stEntry.sLanguage.c_str(), stEntry.sText.c_str() };
        Load(std::span<const MGTextOverride>(&stOverride, 1), true, s_mMG2Text);
    }
    if (!MGTextExtended::vCharacters.empty())
    {
        if (WidenMG2MessageRows())
        {
            spdlog::info("MG2: Text Overrides : Message rows and text items widened.");
        }
        else
        {
            spdlog::error("MG2: Text Overrides : Could not widen the message rows and text items. Long rows with many extra characters will be corrupted.");
        }
    }
    if (s_mMG2Text.empty())
    {
        spdlog::info("MG2: Text Overrides : No replacements configured.");
        return;
    }
    if (!MG2_Gamefuncs::pfnAVS_GetLocalizedString || !Shared_Gamefuncs::pfnGetLanguage)
    {
        spdlog::error("MG2: Text Overrides : Required functions unavailable.");
        return;
    }
    s_hMG2Text = safetyhook::create_inline(MG2_Gamefuncs::pfnAVS_GetLocalizedString, GetLocalizedString);
    LOG_HOOK(s_hMG2Text, "MG2: Text Overrides : AVS_GetLocalizedString()")
    if (s_hMG2Text)
    {
        spdlog::info("MG2: Text Overrides : Initialized.");
    }
}
