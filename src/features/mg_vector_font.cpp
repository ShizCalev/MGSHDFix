#include "stdafx.h"
#include "mg_vector_font.hpp"

#include "common.hpp"
#include "d3d11_api.hpp"
#include "gamevars.hpp"
#include "mg1_linkvarbuf.hpp"
#include "mg2_linkvarbuf.hpp"
#include "mg_font_bundle.hpp"

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

namespace
{
    struct Settings
    {
        const char* pszGame;
        int* pTextFade;
        MG_Gamefuncs::DrawText_t pfnDrawText;
        MG_Gamefuncs::ComposeTextLayer_t pfnComposeTextLayer;
        MG_Gamefuncs::MeasureTextFont_t pfnMeasureTextFont;
        MG_Gamefuncs::UploadTexture_t pfnUploadTexture;
    };

    constexpr std::array<int, 4> anFontHeights = { 20, 12, 18, 24 };
    constexpr int nLayerWidth = 512;
    constexpr int nLayerHeight = 424;
    constexpr size_t nLayerPixels = nLayerWidth * nLayerHeight;
    constexpr size_t nMaximumTextBytes = 4096;
    constexpr size_t nMaximumCachedGlyphs = 4096;

    struct GameRenderer
    {
        Settings stSettings {};
        SafetyHookInline hDrawText;
        SafetyHookInline hCompose;
    };

    std::array<GameRenderer, 2> s_astGames;
    thread_local GameRenderer* s_pGame = nullptr;
    SafetyHookInline s_hUpload;
    SafetyHookInline s_hSetResources;
    std::vector<uint8_t> s_vFontData;
    stbtt_fontinfo s_stFont {};

    struct Glyph
    {
        std::vector<uint8_t> vCoverage;
        int nWidth = 0;
        int nHeight = 0;
        int nX = 0;
        int nY = 0;
    };

    struct Letter
    {
        int nCodepoint;
        int nX;
        int nAdvance;
    };

    struct Texture
    {
        ComPtr<ID3D11ShaderResourceView> pOriginal;
        ComPtr<ID3D11Texture2D> pTexture;
        ComPtr<ID3D11ShaderResourceView> pView;
        int nScale = 0;
        bool bReady = false;
    };

    std::mutex s_stTextureMutex;
    std::vector<Texture> s_vTextures;
    std::unordered_map<uint64_t, Glyph> s_stGlyphs;
    std::vector<uint32_t> s_vPixels;
    uint32_t* s_pFrame = nullptr;
    int s_nScale = 0;
    int s_nFade = 255;
    thread_local bool s_bComposing = false;
    bool s_bRasterFailed = false;

    void __stdcall SetResources_hook(ID3D11DeviceContext* pContext, UINT nStart, UINT nCount, ID3D11ShaderResourceView* const* ppViews)
    {
        if (!ppViews || nCount > D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT)
        {
            s_hSetResources.call<void>(pContext, nStart, nCount, ppViews);
            return;
        }
        std::array<ID3D11ShaderResourceView*, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT> apViews {};
        std::array<ComPtr<ID3D11ShaderResourceView>, D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT> apKeepAlive;
        {
            std::lock_guard<std::mutex> stLock(s_stTextureMutex);
            for (UINT nIndex = 0; nIndex < nCount; nIndex++)
            {
                apViews[nIndex] = ppViews[nIndex];
                for (const Texture& stTexture : s_vTextures)
                {
                    if (stTexture.bReady && stTexture.pOriginal.Get() == ppViews[nIndex])
                    {
                        apKeepAlive[nIndex] = stTexture.pView;
                        apViews[nIndex] = apKeepAlive[nIndex].Get();
                        break;
                    }
                }
            }
        }
        s_hSetResources.call<void>(pContext, nStart, nCount, apViews.data());
    }

    bool PrepareRenderer()
    {
        if (!g_D3D11Hooks.d3dDevice || !g_D3D11Hooks.d3dDeviceContext || !g_D3D11Hooks.swapChain)
        {
            return false;
        }
        if (!s_hSetResources)
        {
            void** ppTable = *reinterpret_cast<void***>(g_D3D11Hooks.d3dDeviceContext.Get());
            s_hSetResources = safetyhook::create_inline(ppTable[8], SetResources_hook);
            LOG_HOOK(s_hSetResources, "MG1/MG2: Vector Font : ID3D11DeviceContext::PSSetShaderResources()")
            if (!s_hSetResources)
            {
                return false;
            }
        }
        DXGI_SWAP_CHAIN_DESC stDesc {};
        if (FAILED(g_D3D11Hooks.swapChain->GetDesc(&stDesc)))
        {
            return false;
        }
        const int nScale = std::clamp(static_cast<int>(std::ceil((std::max)(stDesc.BufferDesc.Width * 0.75f / nLayerWidth, float(stDesc.BufferDesc.Height) / nLayerHeight))), 2, 8);
        if (nScale != s_nScale)
        {
            s_nScale = nScale;
            s_stGlyphs.clear();
            s_vPixels.resize(nLayerPixels * nScale * nScale);
        }
        return true;
    }

    void CopyPixel(size_t nIndex, uint32_t nPixel)
    {
        const size_t nX = (nIndex % nLayerWidth) * s_nScale;
        const size_t nY = (nIndex / nLayerWidth) * s_nScale;
        const size_t nStride = nLayerWidth * s_nScale;
        for (int nRow = 0; nRow < s_nScale; nRow++)
        {
            std::fill_n(s_vPixels.begin() + (nY + nRow) * nStride + nX, s_nScale, nPixel);
        }
    }

    bool DecodeLetters(char* pszText, int nFont, int nX, std::vector<Letter>& vLetters)
    {
        const size_t nLength = strnlen(pszText, nMaximumTextBytes);
        if (nLength == nMaximumTextBytes)
        {
            return false;
        }
        for (size_t nIndex = 0; nIndex < nLength;)
        {
            const auto nChar = static_cast<uint8_t>(pszText[nIndex]);
            if (nChar == '|')
            {
                if (nIndex + 1 == nLength)
                {
                    break;
                }
                nX += 24; // pass controller icons natively.
                nIndex += 2;
                continue;
            }
            const bool bWide = nFont == 0 && nChar >= 0x80 && static_cast<uint8_t>(nChar + 0x60) > 0x3F;
            if (nFont != 0 && nChar >= 0x80 && static_cast<uint8_t>(nChar + 0x60) > 0x3F)
            {
                return false; // these slots use a different byte-to-glyph mapping.
            }
            const int nBytes = bWide ? 2 : 1;
            if (nIndex + nBytes > nLength)
            {
                return false;
            }
            wchar_t awCodepoint[2] {};
            if (MultiByteToWideChar(bWide ? 932 : 1252, MB_ERR_INVALID_CHARS, pszText + nIndex, nBytes, awCodepoint, 2) != 1
                || stbtt_FindGlyphIndex(&s_stFont, awCodepoint[0]) == 0)
            {
                return false;
            }
            const int nAdvance = bWide ? 24 : static_cast<int>(s_pGame->stSettings.pfnMeasureTextFont(pszText, static_cast<int>(nIndex), nBytes, nFont));
            if (nAdvance < 0 || nAdvance > 64)
            {
                return false;
            }
            vLetters.push_back({ awCodepoint[0], nX, nAdvance });
            nX += nAdvance;
            nIndex += nBytes;
        }
        return true;
    }

    const Glyph& GetGlyph(const Letter& stLetter, int nFont)
    {
        const uint64_t nKey = uint64_t(stLetter.nCodepoint) | (uint64_t(stLetter.nAdvance) << 32) | (uint64_t(nFont) << 40);
        const auto stFound = s_stGlyphs.find(nKey);
        if (stFound != s_stGlyphs.end())
        {
            return stFound->second;
        }
        if (s_stGlyphs.size() >= nMaximumCachedGlyphs)
        {
            s_stGlyphs.clear();
        }
        Glyph stGlyph;
        int nAdvance = 0;
        int nBearing = 0;
        int nAscent = 0;
        int nDescent = 0;
        int nLineGap = 0;
        stbtt_GetCodepointHMetrics(&s_stFont, stLetter.nCodepoint, &nAdvance, &nBearing);
        stbtt_GetFontVMetrics(&s_stFont, &nAscent, &nDescent, &nLineGap);
        const float fScaleY = stbtt_ScaleForPixelHeight(&s_stFont, float(anFontHeights[nFont] * s_nScale));
        const float fScaleX = nAdvance > 0 ? float(stLetter.nAdvance * s_nScale) / nAdvance : fScaleY;
        unsigned char* pBitmap = stbtt_GetCodepointBitmap(&s_stFont, fScaleX, fScaleY, stLetter.nCodepoint,
            &stGlyph.nWidth, &stGlyph.nHeight, &stGlyph.nX, &stGlyph.nY);
        stGlyph.nY += static_cast<int>(std::round(nAscent * fScaleY));
        if (pBitmap)
        {
            stGlyph.vCoverage.assign(pBitmap, pBitmap + size_t(stGlyph.nWidth) * stGlyph.nHeight);
            stbtt_FreeBitmap(pBitmap, nullptr);
        }
        else if (stGlyph.nWidth > 0 && stGlyph.nHeight > 0)
        {
            s_bRasterFailed = true;
        }
        return s_stGlyphs.emplace(nKey, std::move(stGlyph)).first->second;
    }

    uint32_t Blend(uint32_t nDest, uint32_t nColor, unsigned int nCoverage)
    {
        const unsigned int nDestAlpha = 255 - (nDest >> 24);
        const unsigned int nAlpha = nCoverage * 255 + nDestAlpha * (255 - nCoverage);
        if (nAlpha == 0)
        {
            return nDest;
        }
        uint32_t nResult = (255 - (nAlpha + 127) / 255) << 24;
        for (int nShift = 0; nShift <= 16; nShift += 8)
        {
            const unsigned int nSource = (nColor >> nShift) & 255;
            const unsigned int nTarget = (nDest >> nShift) & 255;
            nResult |= ((nSource * nCoverage * 255 + nTarget * nDestAlpha * (255 - nCoverage)) / nAlpha) << nShift;
        }
        return nResult;
    }

    void DrawLetter(const Letter& stLetter, int nFont, int nY, uint32_t nColor)
    {
        const Glyph& stGlyph = GetGlyph(stLetter, nFont);
        if (stGlyph.vCoverage.empty())
        {
            return;
        }
        const int nWidth = nLayerWidth * s_nScale;
        const int nHeight = nLayerHeight * s_nScale;
        for (int nRow = 0; nRow < stGlyph.nHeight; nRow++)
        {
            const int nDestY = nY * s_nScale + stGlyph.nY + nRow;
            if (nDestY < 0 || nDestY >= nHeight)
            {
                continue;
            }
            for (int nColumn = 0; nColumn < stGlyph.nWidth; nColumn++)
            {
                const int nDestX = stLetter.nX * s_nScale + stGlyph.nX + nColumn;
                if (nDestX >= 0 && nDestX < nWidth)
                {
                    uint32_t& nDest = s_vPixels[size_t(nDestY) * nWidth + nDestX];
                    nDest = Blend(nDest, nColor, stGlyph.vCoverage[size_t(nRow) * stGlyph.nWidth + nColumn]);
                }
            }
        }
    }

    template <size_t nGame>
    void __fastcall DrawText_hook(uint32_t* pFrame, int* pText)
    {
        GameRenderer& stGame = s_astGames[nGame];
        if (!s_bComposing || s_pGame != &stGame)
        {
            stGame.hDrawText.call<void>(pFrame, pText);
            return;
        }
        if (!s_pFrame)
        {
            s_pFrame = pFrame;
            for (size_t nIndex = 0; nIndex < nLayerPixels; nIndex++)
            {
                CopyPixel(nIndex, pFrame[nIndex]);
            }
        }
        const int nX = pText[1];
        const int nY = pText[2];
        const int nFont = pText[3];
        const uint32_t nColor = pText[0];
        std::vector<Letter> vLetters;
        const bool bSupported = nFont >= 0 && nFont < 4 && DecodeLetters(reinterpret_cast<char*>(pText + 4), nFont, nX, vLetters);
        // keep the original font rendering layer in case we have to fall back easily.
        stGame.hDrawText.call<void>(pFrame, pText);
        if (!bSupported)
        {
            s_bRasterFailed = true;
        }
        else if (pText[1] != nX)
        {
            for (const Letter& stLetter : vLetters)
            {
                DrawLetter(stLetter, nFont, nY, nColor);
            }
        }
    }

    void UploadReplacement(ID3D11ShaderResourceView* pOriginal)
    {
        ComPtr<ID3D11Resource> pResource;
        ComPtr<ID3D11Texture2D> pOriginalTexture;
        pOriginal->GetResource(&pResource);
        if (FAILED(pResource.As(&pOriginalTexture)))
        {
            return;
        }
        D3D11_TEXTURE2D_DESC stDesc {};
        pOriginalTexture->GetDesc(&stDesc);
        if (stDesc.Width != nLayerWidth || stDesc.Height != nLayerHeight || stDesc.SampleDesc.Count != 1
            || (stDesc.Format != DXGI_FORMAT_R8G8B8A8_UNORM && stDesc.Format != DXGI_FORMAT_B8G8R8A8_UNORM))
        {
            return;
        }
        std::lock_guard<std::mutex> stLock(s_stTextureMutex);
        auto stFound = std::find_if(s_vTextures.begin(), s_vTextures.end(), [pOriginal](const Texture& stTexture) { return stTexture.pOriginal.Get() == pOriginal; });
        if (stFound == s_vTextures.end())
        {
            if (s_vTextures.size() >= 8)
            {
                s_vTextures.clear();
            }
            s_vTextures.emplace_back();
            stFound = s_vTextures.end() - 1;
            stFound->pOriginal = pOriginal;
        }
        Texture& stTexture = *stFound;
        stTexture.bReady = false;
        if (stTexture.nScale != s_nScale)
        {
            stTexture.pView.Reset();
            stTexture.pTexture.Reset();
            stTexture.nScale = 0;
            stDesc.Width *= s_nScale;
            stDesc.Height *= s_nScale;
            stDesc.MipLevels = 1;
            stDesc.ArraySize = 1;
            stDesc.Usage = D3D11_USAGE_DEFAULT;
            stDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            stDesc.CPUAccessFlags = 0;
            stDesc.MiscFlags = 0;
            if (FAILED(g_D3D11Hooks.d3dDevice->CreateTexture2D(&stDesc, nullptr, &stTexture.pTexture))
                || FAILED(g_D3D11Hooks.d3dDevice->CreateShaderResourceView(stTexture.pTexture.Get(), nullptr, &stTexture.pView)))
            {
                return;
            }
            stTexture.nScale = s_nScale;
        }
        for (uint32_t& nPixel : s_vPixels)
        {
            uint32_t nColor = 0;
            for (int nShift = 0; nShift <= 16; nShift += 8)
            {
                nColor |= ((((nPixel >> nShift) & 255) * s_nFade) / 255) << nShift;
            }
            nPixel = nColor | ((nPixel ^ 0xFF000000u) & 0xFF000000u);
        }
        g_D3D11Hooks.d3dDeviceContext->UpdateSubresource(stTexture.pTexture.Get(), 0, nullptr, s_vPixels.data(), nLayerWidth * s_nScale * sizeof(uint32_t), 0);
        stTexture.bReady = true;
    }

    int64_t __fastcall UploadTexture_hook(void* pTexture, uint32_t* pPixels)
    {
        const int64_t nResult = s_hUpload.call<int64_t>(pTexture, pPixels);
        if (s_bComposing && s_pFrame == pPixels && !s_bRasterFailed)
        {
            auto* pHostTexture = static_cast<MGHostTexture*>(pTexture);
            if (pHostTexture->pShaderResourceView)
            {
                UploadReplacement(pHostTexture->pShaderResourceView);
            }
        }
        return nResult;
    }

    template <size_t nGame>
    void __fastcall Compose_hook(int nDrawText)
    {
        GameRenderer& stGame = s_astGames[nGame];
        s_pGame = &stGame;
        {
            std::lock_guard<std::mutex> stLock(s_stTextureMutex);
            for (Texture& stTexture : s_vTextures)
            {
                stTexture.bReady = false;
            }
        }
        s_pFrame = nullptr;
        s_bRasterFailed = false;
        s_nFade = std::clamp(*stGame.stSettings.pTextFade, 0, 255);
        s_bComposing = MG_VectorFont::bEnabled && nDrawText != 0 && PrepareRenderer();
        stGame.hCompose.call<void>(nDrawText);
        s_bComposing = false;
        s_pFrame = nullptr;
        s_pGame = nullptr;
    }

    bool LoadFont()
    {
        const auto stPath = sExePath / "launcher_Data/StreamingAssets/aa/StandaloneWindows64/defaultlocalgroup_assets_launcher/fonts/mg-rodinpron-m.otf.bundle";
        std::ifstream stFile(stPath, std::ios::binary | std::ios::ate);
        if (!stFile)
        {
            return false;
        }
        const auto nSize = stFile.tellg();
        if (nSize <= 0 || static_cast<uint64_t>(nSize) > MG_FontData::nMaximumBytes)
        {
            return false;
        }
        std::vector<uint8_t> vBundle(static_cast<size_t>(nSize));
        stFile.seekg(0);
        if (!stFile.read(reinterpret_cast<char*>(vBundle.data()), nSize) || !MG_FontData::Unpack(vBundle, s_vFontData))
        {
            return false;
        }
        return stbtt_InitFont(&s_stFont, s_vFontData.data(), 0) != 0;
    }

    void Initialize(size_t nGame, const Settings& stSettings)
    {
        if (nGame >= s_astGames.size() || !MG_VectorFont::bEnabled || s_astGames[nGame].hCompose)
        {
            return;
        }
        if (s_vFontData.empty() && !LoadFont())
        {
            s_vFontData.clear();
            spdlog::error("{}: Vector Font : Unable to parse font file from the unity launcher's files. Aborting setup!", stSettings.pszGame);
            return;
        }
        if (!stSettings.pfnDrawText || !stSettings.pfnComposeTextLayer || !stSettings.pfnMeasureTextFont || !stSettings.pfnUploadTexture || !stSettings.pTextFade)
        {
            spdlog::error("{}: Vector Font : Couldn't find text renderer. Aborting setup.", stSettings.pszGame);
        }
        else
        {
            GameRenderer& stGame = s_astGames[nGame];
            stGame.stSettings = stSettings;
            if (!s_hUpload)
            {
                s_hUpload = safetyhook::create_inline(stSettings.pfnUploadTexture, UploadTexture_hook);
                LOG_HOOK(s_hUpload, "MG1/MG2: Vector Font : sub_140025F80() : upload")
            }
            stGame.hDrawText = safetyhook::create_inline(stSettings.pfnDrawText, nGame == 0 ? DrawText_hook<0> : DrawText_hook<1>);
            LOG_HOOK(stGame.hDrawText, std::string(stSettings.pszGame) + ": Vector Font : draw text")
            stGame.hCompose = safetyhook::create_inline(stSettings.pfnComposeTextLayer, nGame == 0 ? Compose_hook<0> : Compose_hook<1>);
            LOG_HOOK(stGame.hCompose, std::string(stSettings.pszGame) + ": Vector Font : compose")
            if (!stGame.hDrawText || !s_hUpload || !stGame.hCompose)
            {
                stGame.hCompose.reset();
                stGame.hDrawText.reset();
                if (!s_astGames[0].hCompose && !s_astGames[1].hCompose)
                {
                    s_hUpload.reset();
                }
                spdlog::error("{}: Vector Font : Missing hook! Setup aborted.", stSettings.pszGame);
            }
        }
    }
}

void MG_VectorFont::InitializeMG1()
{
    Initialize(0, { "MG1", MG1_LinkVarBuf::pTextFade, MG1_Gamefuncs::pfnDrawTextSlot, MG1_Gamefuncs::pfnComposeTextLayer, MG1_Gamefuncs::pfnMeasureTextFont, MG1_Gamefuncs::pfnUploadTexture });
}

void MG_VectorFont::InitializeMG2()
{
    Initialize(1, { "MG2", MG2_LinkVarBuf::pTextFade, MG2_Gamefuncs::pfnDrawTextItem, MG2_Gamefuncs::pfnComposeTextLayer, MG2_Gamefuncs::pfnMeasureTextFont, MG2_Gamefuncs::pfnUploadTexture });
}
