#include "stdafx.h"
#include "mg2_ending_cast.hpp"

#include "common.hpp"
#include "game_funcs.hpp"
#include "mg_text_overrides.hpp"
#include "mg2_gamevar_enums.hpp"
#include "mg2_linkvarbuf.hpp"
#include "mg2_ending_cast_data.hpp"
#include "mg2_ending_timing.hpp"

using namespace MG2_Gamefuncs;

namespace
{
    SafetyHookInline s_hGetChunkEntryData;
    bool s_bWarned = false;
    bool s_bTimingWarned = false;

    constexpr bool bNamesEnglishOnly = true; //Restored character names - will change this once we add all the other languages... (hopefully...)

    constexpr size_t nSheetHeaderSize = 8;
    constexpr size_t nRecordSize = 10;
    constexpr size_t nRecordDurationOffset = 8;

    [[nodiscard]] uint16_t ReadU16(const uint8_t* pData)
    {
        return static_cast<uint16_t>((pData[0] << 8) | pData[1]);
    }

    void WriteU16(uint8_t* pData, int nValue)
    {
        pData[0] = static_cast<uint8_t>(nValue >> 8);
        pData[1] = static_cast<uint8_t>(nValue);
    }

    void RestoreNames(uint8_t* pData)
    {
        const auto& stFirst = MG2_EndingCastData::astChanges[0];
        if (pData[stFirst.nOffset] != stFirst.nOriginal)
        {
            return;
        }
        uint64_t nHash = 0xCBF29CE484222325ull;
        for (size_t nIndex = 0; nIndex < MG2_EndingCastData::nSize; ++nIndex)
        {
            nHash = (nHash ^ pData[nIndex]) * 0x100000001B3ull;
        }
        if (nHash != MG2_EndingCastData::nOriginalHash)
        {
            if (!s_bWarned)
            {
                spdlog::warn("MG2: Ending Cast : Unexpected animation data. Skipping restoration.");
                s_bWarned = true;
            }
            return;
        }
        for (const auto& stChange : MG2_EndingCastData::astChanges)
        {
            pData[stChange.nOffset] = stChange.nRestored;
        }
    }

    void RestoreTiming(uint8_t* pData)
    {
        const size_t nTracks = ReadU16(pData + 6);
        int nLastTick = 0;
        size_t nOffset = nSheetHeaderSize;
        for (size_t nTrack = 0; nTrack < nTracks; nTrack++)
        {
            if (nOffset + 2 > MG2_EndingCastData::nSize)
            {
                return;
            }
            const size_t nCount = ReadU16(pData + nOffset);
            nOffset += 2;
            if (nOffset + nCount * nRecordSize > MG2_EndingCastData::nSize)
            {
                return;
            }
            int nTick = 0;
            for (size_t nRecord = 0; nRecord < nCount; nRecord++)
            {
                nTick += ReadU16(pData + nOffset + nRecord * nRecordSize + nRecordDurationOffset);
            }
            nLastTick = std::max(nLastTick, nTick);
            nOffset += nCount * nRecordSize;
        }
        if (nLastTick != MG2_EndingTiming::nVanillaLastTick)
        {
            if (nLastTick != MG2_EndingTiming::nRestoredLastTick && !s_bTimingWarned)
            {
                spdlog::warn("MG2: Ending Timing : Unexpected cast roll length {}. Keeping MC timing.", nLastTick);
                s_bTimingWarned = true;
            }
            return;
        }

        nOffset = nSheetHeaderSize;
        for (size_t nTrack = 0; nTrack < nTracks; nTrack++)
        {
            const size_t nCount = ReadU16(pData + nOffset);
            nOffset += 2;
            int nTick = 0;
            int nRestoredTick = 0;
            for (size_t nRecord = 0; nRecord < nCount; nRecord++)
            {
                uint8_t* pDuration = pData + nOffset + nRecord * nRecordSize + nRecordDurationOffset;
                const int nDuration = ReadU16(pDuration);
                nTick += nDuration;
                const int nEnd = nDuration == 0 ? nRestoredTick : std::max(MG2_EndingTiming::RetimeTick(nTick), nRestoredTick + 1);
                WriteU16(pDuration, nEnd - nRestoredTick);
                nRestoredTick = nEnd;
            }
            nOffset += nCount * nRecordSize;
        }
        spdlog::info("MG2: Ending Timing : Cast roll retimed, {} -> {} ticks.", nLastTick, MG2_EndingTiming::nRestoredLastTick);
    }

    // Script 142
    void RestoreMusicCue()
    {
        if (!MG2_LinkVarBuf::pLoadedScripts || !pfnGetHandleData || MG2_LinkVarBuf::pLoadedScripts[MG2_SCRIPT_ENDING_CAST] == 0)
        {
            return;
        }
        auto* pCode = static_cast<uint8_t*>(pfnGetHandleData(MG2_LinkVarBuf::pLoadedScripts[MG2_SCRIPT_ENDING_CAST]));
        constexpr size_t nCueSize = 2;
        if (!Memory::IsReadable(pCode, MG2_EndingTiming::nCueSizeOffset + 1 + nCueSize) || pCode[MG2_EndingTiming::nCueSizeOffset] != nCueSize)
        {
            return;
        }
        uint8_t* pCue = pCode + MG2_EndingTiming::nCueSizeOffset + 1;
        if (ReadU16(pCue) == MG2_EndingTiming::nVanillaCueTick)
        {
            WriteU16(pCue, MG2_EndingTiming::nRestoredCueTick);
            spdlog::info("MG2: Ending Timing : Music fade cue {} -> {}.", MG2_EndingTiming::nVanillaCueTick, MG2_EndingTiming::nRestoredCueTick);
        }
    }

    uint8_t* __fastcall GetChunkEntryData_hook(uintptr_t hChunk, int nEntry)
    {
        auto* pData = s_hGetChunkEntryData.call<uint8_t*>(hChunk, nEntry);
        if (nEntry != MG2_SPRITE_SHEET_ENDING_CAST || !pData || pfnGetChunkEntrySize(hChunk, nEntry) != MG2_EndingCastData::nSize)
        {
            return pData;
        }
        const bool bEnglish = Shared_Gamefuncs::pfnGetLanguage && Shared_Gamefuncs::pfnGetLanguage() == static_cast<int>(MGLanguage::English);
        if (MG_TextOverrides::bMG2RestoreOriginalScript && (!bNamesEnglishOnly || bEnglish))
        {
            RestoreNames(pData);
        }
        if (MG2_EndingTiming::bEnabled)
        {
            RestoreTiming(pData);
            RestoreMusicCue();
        }
        return pData;
    }
}

void MG2_EndingCast::Initialize()
{
    if (!MG_TextOverrides::bMG2RestoreOriginalScript && !MG2_EndingTiming::bEnabled)
    {
        return;
    }
    if (!pfnGetChunkEntryData || !pfnGetChunkEntrySize)
    {
        spdlog::error("MG2: Ending Cast : Required resource functions unavailable.");
    }
    else
    {
        s_hGetChunkEntryData = safetyhook::create_inline(pfnGetChunkEntryData, GetChunkEntryData_hook);
        LOG_HOOK(s_hGetChunkEntryData, "MG2: Ending Cast : GetChunkEntryData()")
        if (s_hGetChunkEntryData)
        {
            spdlog::info("MG2: Ending Cast : Initialized.");
        }
    }
}
