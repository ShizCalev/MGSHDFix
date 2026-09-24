#include "stdafx.h"

#include "mgs2_title_lightning.hpp"
#include "d3d11_api.hpp"
#include "common.hpp"
#include "gamevars.hpp"

#include "logging.hpp"

#include <atomic>

// L2 on the title calls down lightning: a grey sheet flips the picture to a negative for a few frames.
// It asks for double strength, which D3D11 flattens back to one, so the sheet just paints the title grey.

namespace
{
    // the sheet fades out through half strength, so draw it twice and let each pass keep only its own pixels
    const char* kPlateShader = R"(
    struct PSIn
    {
        float4 pos : SV_POSITION;
        float4 col : TEXCOORD1;
    };
    float4 plain(PSIn i) : SV_Target
    {
        clip(128.5 / 255.0 - i.col.a);
        return i.col * float4(1, 1, 1, 2);
    }
    float4 overbright(PSIn i) : SV_Target
    {
        clip(i.col.a - 128.5 / 255.0);
        float k = i.col.a * (255.0 / 128.0);
        return float4(saturate(i.col.rgb * k), saturate(2.0 - k));
    }
    )";

    constexpr UINT kQuadVerts    = 4;
    constexpr UINT kPosColStride = 32;   // position and colour only: no texture to lose when we swap the shader

    ComPtr<ID3DBlob>            plainBlob;
    ComPtr<ID3DBlob>            overbrightBlob;
    ComPtr<ID3D11PixelShader>   psPlain;
    ComPtr<ID3D11PixelShader>   psOverbright;
    ComPtr<ID3D11BlendState>    blendOverbright;

    safetyhook::InlineHook Draw_hook;

    std::atomic<bool> bOnTitle{ false };

    bool IsPlate(ID3D11DeviceContext* context, UINT vertexCount)
    {
        if (vertexCount != kQuadVerts)
        {
            return false;
        }

        D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
        context->IAGetPrimitiveTopology(&topology);
        if (topology != D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP)
        {
            return false;
        }

        ID3D11Buffer* vb = nullptr;
        UINT stride = 0;
        UINT offset = 0;
        context->IAGetVertexBuffers(0, 1, &vb, &stride, &offset);
        if (vb)
        {
            vb->Release();
        }
        if (stride != kPosColStride)
        {
            return false;
        }

        ID3D11BlendState* blend = nullptr;
        float factor[4] = {};
        UINT mask = 0;
        context->OMGetBlendState(&blend, factor, &mask);
        if (!blend)
        {
            return false;
        }

        D3D11_BLEND_DESC desc = {};
        blend->GetDesc(&desc);
        blend->Release();

        const auto& rt = desc.RenderTarget[0];
        return rt.BlendEnable
            && rt.SrcBlend == D3D11_BLEND_SRC_ALPHA
            && rt.DestBlend == D3D11_BLEND_INV_SRC_ALPHA
            && rt.BlendOp == D3D11_BLEND_OP_ADD;
    }

    void __stdcall HookedDraw(ID3D11DeviceContext* context, UINT vertexCount, UINT startVertexLocation)
    {
        if (!bOnTitle.load(std::memory_order_relaxed) || !IsPlate(context, vertexCount))
        {
            Draw_hook.call<void>(context, vertexCount, startVertexLocation);
            return;
        }

        ID3D11BlendState* oldBlend = nullptr;
        ID3D11PixelShader* oldPS = nullptr;
        float oldFactor[4] = {};
        UINT oldMask = 0;

        context->OMGetBlendState(&oldBlend, oldFactor, &oldMask);
        context->PSGetShader(&oldPS, nullptr, nullptr);

        context->PSSetShader(psPlain.Get(), nullptr, 0);
        Draw_hook.call<void>(context, vertexCount, startVertexLocation);

        // the extra brightness has to ride in the colour; anywhere else D3D11 flattens it again
        context->OMSetBlendState(blendOverbright.Get(), oldFactor, oldMask);
        context->PSSetShader(psOverbright.Get(), nullptr, 0);
        Draw_hook.call<void>(context, vertexCount, startVertexLocation);

        context->OMSetBlendState(oldBlend, oldFactor, oldMask);
        context->PSSetShader(oldPS, nullptr, 0);

        if (oldBlend)
        {
            oldBlend->Release();
        }

        if (oldPS)
        {
            oldPS->Release();
        }
    }

    bool Compile(const char* entry, ComPtr<ID3DBlob>& blob)
    {
        ComPtr<ID3DBlob> err;
        const HRESULT hr = g_D3D11Hooks.D3DCompileFunc(kPlateShader, strlen(kPlateShader), nullptr, nullptr,
            nullptr, entry, "ps_5_0", 0, 0, blob.ReleaseAndGetAddressOf(), err.ReleaseAndGetAddressOf());
        if (FAILED(hr))
        {
            spdlog::error("MGS 2: Title Lightning: Failed to compile pixel shader {}: {}", entry,
                err ? static_cast<const char*>(err->GetBufferPointer()) : "Unknown error");
            return false;
        }
        return true;
    }
}

void MGS2_TitleLightning::Setup()
{
    if (!(eGameType & MGS2))
    {
        return;
    }

    if (!bEnabled)
    {
        spdlog::info("MGS 2: Title Lightning: Disabled in config. Skipping");
        return;
    }

    if (!g_D3D11Hooks.D3DCompileFunc)
    {
        spdlog::error("MGS 2: Title Lightning: Failed to get D3DCompile.");
        return;
    }

    if (Compile("plain", plainBlob) && Compile("overbright", overbrightBlob))
    {
        spdlog::info("MGS 2: Title Lightning: Compiled pixel shaders successfully.");
    }
}

void MGS2_TitleLightning::Init()
{
    if (!(eGameType & MGS2) || !bEnabled || bLoaded)
    {
        return;
    }

    ID3D11Device* dev = g_D3D11Hooks.d3dDevice.Get();
    if (!dev || !plainBlob || !overbrightBlob)
    {
        return;
    }

    if (FAILED(dev->CreatePixelShader(plainBlob->GetBufferPointer(), plainBlob->GetBufferSize(), nullptr, psPlain.GetAddressOf()))
        || FAILED(dev->CreatePixelShader(overbrightBlob->GetBufferPointer(), overbrightBlob->GetBufferSize(), nullptr, psOverbright.GetAddressOf())))
    {
        spdlog::error("MGS 2: Title Lightning: Failed to create pixel shaders.");
        return;
    }

    D3D11_BLEND_DESC bd = {};
    auto& rt = bd.RenderTarget[0];
    rt.BlendEnable = TRUE;
    rt.SrcBlend = D3D11_BLEND_ONE;
    rt.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    rt.BlendOp = D3D11_BLEND_OP_SUBTRACT;
    rt.SrcBlendAlpha = D3D11_BLEND_ZERO;
    rt.DestBlendAlpha = D3D11_BLEND_ONE;
    rt.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    rt.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    if (FAILED(dev->CreateBlendState(&bd, blendOverbright.GetAddressOf())))
    {
        spdlog::error("MGS 2: Title Lightning: Failed to create blend state.");
        return;
    }

    if (!Draw_hook)
    {
        void** vtable = *reinterpret_cast<void***>(g_D3D11Hooks.d3dDeviceContext.Get());
        Draw_hook = safetyhook::create_inline(vtable[13], reinterpret_cast<void*>(HookedDraw));
        LOG_HOOK(Draw_hook, "MGS 2: Title Lightning: ID3D11DeviceContext::Draw")
    }

    plainBlob.Reset();
    overbrightBlob.Reset();

    bLoaded = true;
    spdlog::info("MGS 2: Title Lightning initialized.");
}

void MGS2_TitleLightning::HandleLevelTransition()
{
    if (!(eGameType & MGS2) || !bEnabled)
    {
        return;
    }

    bOnTitle.store(g_GameVars.IsStage(MGS2Stages::N_TITLE), std::memory_order_relaxed);
}
