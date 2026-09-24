#include "stdafx.h"
#include "mgs2_thermal_heat.hpp"

#include "common.hpp"
#include "logging.hpp"
#include "d3d11_api.hpp"
#include "mgs2_blood_stains.hpp"

#include <algorithm>

using Microsoft::WRL::ComPtr;

namespace
{
    // BP_Renderer.cpp render command ids
    constexpr int kCmd_Obj_LocalParam = 32;
    constexpr int kCmd_Obj_Render = 33;
    constexpr int kCmd_Obj_RenderMulti = 34;
    constexpr int kCmd_Evm_LocalParam = 52;
    constexpr int kCmd_Evm_Render = 53;
    constexpr int kCmd_Evm_RenderMulti = 54;

    constexpr uint32_t DG_FLAG_PAINT = 0x0001;
    constexpr uint32_t DG_FLAG_IRREACTION = 0x0100;
    constexpr uint32_t DG_STATE_IR_MODE = 0x0001;

    // The PS2 clamps each vertex's lit colour to white before drawing; the port never does.
    // These shaders redo the stock vertex maths and add that clamp.
    constexpr const char* kHeatTail = R"(
float4 HeatCol(float3 n)
{
    float4 d = float4(max(float3(dot(g[2].xyz, n), dot(g[3].xyz, n), dot(g[4].xyz, n)), 0.0), 1.0);
    return float4(saturate(float3(dot(g[6], d), dot(g[7], d), dot(g[8], d)) / 255.0), 1.0);
}
float4 HeatFog(float w)
{
    float f = saturate(w * g[1].x + g[1].y);
    return float4(g[0].xyz, max(min(f, g[1].w), g[1].z));
}
)";

    // Skinned models; each variant reads its uv from a different slot.
    constexpr const char* kHeatSkinnedVS = R"(
cbuffer Globals : register(b0) { float4 g[48]; }
#if UVSET == 0
struct VSIn { float4 pos : POSITION; float2 uv : TEXCOORD0; float3 nrm : TEXCOORD2; int4 bi : BLENDINDICES; float4 bw : BLENDWEIGHT; };
#elif UVSET == 1
struct VSIn { float4 pos : POSITION; float3 nrm : TEXCOORD2; float2 uv : TEXCOORD3; int4 bi : BLENDINDICES; float4 bw : BLENDWEIGHT; };
#else
struct VSIn { float4 pos : POSITION; float3 nrm : TEXCOORD2; int4 bi : BLENDINDICES; float4 bw : BLENDWEIGHT; };
#endif
struct VSOut { float4 pos : SV_POSITION; float2 uv : TEXCOORD0; float4 col : TEXCOORD1; float4 fog : TEXCOORD2; };
TAIL
VSOut main(VSIn i)
{
    int4 j = i.bi * 3 + 24;
    float4 m0 = g[j.x] * i.bw.x + g[j.y] * i.bw.y + g[j.z] * i.bw.z + g[j.w] * i.bw.w;
    float4 m1 = g[j.x + 1] * i.bw.x + g[j.y + 1] * i.bw.y + g[j.z + 1] * i.bw.z + g[j.w + 1] * i.bw.w;
    float4 m2 = g[j.x + 2] * i.bw.x + g[j.y + 2] * i.bw.y + g[j.z + 2] * i.bw.z + g[j.w + 2] * i.bw.w;
    float4 p = float4(i.pos.xyz, 1.0);
    float4 wp = float4(dot(m0, p), dot(m1, p), dot(m2, p), 1.0);
    float3 n = normalize(float3(dot(m0.xyz, i.nrm), dot(m1.xyz, i.nrm), dot(m2.xyz, i.nrm)));
    VSOut o;
    o.pos = float4(dot(g[16], wp), dot(g[17], wp), dot(g[18], wp), dot(g[19], wp));
    o.uv = float2(0.0, 0.0);
    o.fog = HeatFog(o.pos.w);
    o.col = HeatCol(n);
    return o;
}
)";

    // Rigid models, plus the variant whose normals are packed and need scaling up.
    constexpr const char* kHeatRigidVS = R"(
cbuffer Globals : register(b0) { float4 g[34]; }
struct VSIn { float4 pos : POSITION; float2 uv : TEXCOORD0; float3 nrm : TEXCOORD2; };
struct VSOut { float4 pos : SV_POSITION; float2 uv : TEXCOORD0; float4 col : TEXCOORD1; float4 fog : TEXCOORD2; };
TAIL
VSOut main(VSIn i)
{
    bool isShort = asint(g[33].x) == 1;
    float4 p = float4(isShort ? (float3)asint(i.pos.xyz) : i.pos.xyz, 1.0);
    float4 wp = float4(dot(g[16], p), dot(g[17], p), dot(g[18], p), dot(g[19], p));
    float3 nin = (isShort ? (float3)asint(i.nrm) : i.nrm) * NRMSCALE;
    float3 n = float3(dot(g[16].xyz, nin), dot(g[17].xyz, nin), dot(g[18].xyz, nin));
    VSOut o;
    o.pos = float4(dot(g[20], wp), dot(g[21], wp), dot(g[22], wp), dot(g[23], wp));
    o.uv = isShort ? (float2)asint(i.uv) : i.uv;
    o.fog = HeatFog(o.pos.w);
    o.col = HeatCol(n);
    return o;
}
)";

    // Rigid models that morph between two poses; the blend weight is in POSITION.w.
    constexpr const char* kHeatMorphVS = R"(
cbuffer Globals : register(b0) { float4 g[34]; }
struct VSIn { float4 pos : POSITION; float2 uv : TEXCOORD0; float3 nrm : TEXCOORD2; };
struct VSOut { float4 pos : SV_POSITION; float2 uv : TEXCOORD0; float4 col : TEXCOORD1; float4 fog : TEXCOORD2; };
TAIL
VSOut main(VSIn i)
{
    bool isShort = asint(g[33].x) == 1;
    float4 p = float4(isShort ? (float3)asint(i.pos.xyz) : i.pos.xyz, 1.0);
    float t = isShort ? (float)asint(i.pos.w) : i.pos.w;
    float4 a = float4(dot(g[16], p), dot(g[17], p), dot(g[18], p), dot(g[19], p));
    float4 b = float4(dot(g[24], p), dot(g[25], p), dot(g[26], p), dot(g[27], p));
    float4 wp = b + t * (a - b);
    float3 nin = isShort ? (float3)asint(i.nrm) : i.nrm;
    float3 na = float3(dot(g[16].xyz, nin), dot(g[17].xyz, nin), dot(g[18].xyz, nin));
    float3 nb = float3(dot(g[24].xyz, nin), dot(g[25].xyz, nin), dot(g[26].xyz, nin));
    float3 n = normalize(nb + t * (na - nb));
    VSOut o;
    o.pos = float4(dot(g[20], wp), dot(g[21], wp), dot(g[22], wp), dot(g[23], wp));
    o.uv = isShort ? (float2)asint(i.uv) : i.uv;
    o.fog = HeatFog(o.pos.w);
    o.col = HeatCol(n);
    return o;
}
)";

    // On the PS2 the lit colour went straight to the screen, with fog laid on top.
    constexpr const char* kHeatPS = R"(
struct PSIn { float4 pos : SV_POSITION; float2 uv : TEXCOORD0; float4 col : TEXCOORD1; float4 fog : TEXCOORD2; };
float4 main(PSIn i) : SV_TARGET
{
    return float4(lerp(saturate(i.col.rgb * SCALE), i.fog.rgb, i.fog.w), 1.0);
}
)";

    SafetyHookInline gLocalParamHook {};
    SafetyHookInline gObjRenderHook {};
    SafetyHookInline gObjRenderMultiHook {};
    SafetyHookInline gEvmLocalParamHook {};
    SafetyHookInline gEvmRenderHook {};
    SafetyHookInline gEvmRenderMultiHook {};
    SafetyHookInline gDrawIndexedHook {};

    const uint32_t* pDisplayStatus = nullptr;
    bool gGroupIR = false;
    bool gEvmIR = false;
    bool gIRDraw = false;
    bool gIRPaint = false;

    enum HeatVS { kSkinnedUv0, kSkinnedUv1, kSkinnedUv2, kRigid, kRigidShortNrm, kMorph, kHeatVSCount };
    ComPtr<ID3D11VertexShader> gHeatVS[kHeatVSCount];
    ComPtr<ID3D11PixelShader> gHeatLit;       // lit colour, already /255 and clamped
    ComPtr<ID3D11PixelShader> gHeatPaint;     // preshade colour, /128 in the stream
    ComPtr<ID3D11BlendState> gOpaqueBlend;

    // Placed objects only ever set the IR flag on their group, so remember it from there.
    void __fastcall HookedObjLocalParam(uint8_t* packet)
    {
        const uint8_t* objs = *reinterpret_cast<uint8_t* const*>(packet + 8);
        gGroupIR = objs && (*reinterpret_cast<const uint32_t*>(objs + 0x58) & DG_FLAG_IRREACTION);
        gLocalParamHook.fastcall<void>(packet);
    }

    void ObjRender(SafetyHookInline& hook, uint8_t* packet)
    {
        gIRDraw = gGroupIR || *reinterpret_cast<const uint32_t*>(packet + 0x1B8) != 0;
        gIRPaint = (*reinterpret_cast<const uint32_t*>(packet + 0x180) & DG_FLAG_PAINT) != 0;
        hook.fastcall<void>(packet);
        gIRDraw = false;
    }

    void __fastcall HookedObjRender(uint8_t* packet) { ObjRender(gObjRenderHook, packet); }
    void __fastcall HookedObjRenderMulti(uint8_t* packet) { ObjRender(gObjRenderMultiHook, packet); }

    void __fastcall HookedEvmLocalParam(uint8_t* packet)
    {
        gEvmIR = *reinterpret_cast<const uint32_t*>(packet + 0x90) != 0;
        gEvmLocalParamHook.fastcall<void>(packet);
    }

    void EvmRender(SafetyHookInline& hook, uint8_t* packet)
    {
        gIRDraw = gEvmIR;
        gIRPaint = false;
        hook.fastcall<void>(packet);
        gIRDraw = false;
    }

    void __fastcall HookedEvmRender(uint8_t* packet) { EvmRender(gEvmRenderHook, packet); }
    void __fastcall HookedEvmRenderMulti(uint8_t* packet) { EvmRender(gEvmRenderMultiHook, packet); }

    ID3D11VertexShader* HeatVSFor(ID3D11VertexShader* stock)
    {
        switch (D3D11Hooks::GetStockVS(stock))
        {
        case D3D11Hooks::StockVS::KmsLitUv0: return gHeatVS[kSkinnedUv0].Get();
        case D3D11Hooks::StockVS::KmsLitUv1: return gHeatVS[kSkinnedUv1].Get();
        case D3D11Hooks::StockVS::KmsLitUv2: return gHeatVS[kSkinnedUv2].Get();
        case D3D11Hooks::StockVS::KmsLitRigid: return gHeatVS[kRigid].Get();
        case D3D11Hooks::StockVS::KmsLitRigidShortNrm: return gHeatVS[kRigidShortNrm].Get();
        case D3D11Hooks::StockVS::KmsLitMorph: return gHeatVS[kMorph].Get();
        default: return nullptr;
        }
    }

    // Heat draws: no texture, no blending, just the lit colour.
    void STDMETHODCALLTYPE HookedDrawIndexed(ID3D11DeviceContext* ctx, UINT indexCount, UINT startIndex, INT baseVertex)
    {
        // leave Blood Stains' own pass alone, or the whole body gets painted the wrong colour
        if (!(*pDisplayStatus & DG_STATE_IR_MODE) || !gIRDraw || MGS2BloodStains::InOverlay())
        {
            gDrawIndexedHook.stdcall<void>(ctx, indexCount, startIndex, baseVertex);
            return;
        }

        ComPtr<ID3D11VertexShader> theirVS;
        ComPtr<ID3D11PixelShader> theirPS;
        ComPtr<ID3D11BlendState> theirBlend;
        float blendFactor[4] = {};
        UINT sampleMask = 0;
        ctx->VSGetShader(theirVS.GetAddressOf(), nullptr, nullptr);
        ctx->PSGetShader(theirPS.GetAddressOf(), nullptr, nullptr);
        ctx->OMGetBlendState(theirBlend.GetAddressOf(), blendFactor, &sampleMask);

        ID3D11VertexShader* heatVS = gIRPaint ? nullptr : HeatVSFor(theirVS.Get());
        if (heatVS)
        {
            ctx->VSSetShader(heatVS, nullptr, 0);
        }
        ctx->PSSetShader(gIRPaint ? gHeatPaint.Get() : gHeatLit.Get(), nullptr, 0);
        ctx->OMSetBlendState(gOpaqueBlend.Get(), blendFactor, sampleMask);
        gDrawIndexedHook.stdcall<void>(ctx, indexCount, startIndex, baseVertex);
        if (heatVS)
        {
            ctx->VSSetShader(theirVS.Get(), nullptr, 0);
        }
        ctx->PSSetShader(theirPS.Get(), nullptr, 0);
        ctx->OMSetBlendState(theirBlend.Get(), blendFactor, sampleMask);
    }

    ComPtr<ID3DBlob> CompileHeat(const std::string& source, const char* macro, const char* value, const char* profile)
    {
        const D3D_SHADER_MACRO macros[] = { { macro, value }, { nullptr, nullptr } };
        ComPtr<ID3DBlob> blob, err;
        const HRESULT hr = g_D3D11Hooks.D3DCompileFunc(source.c_str(), source.size(), nullptr, macros, nullptr,
            "main", profile, 0, 0, blob.GetAddressOf(), err.GetAddressOf());
        if (FAILED(hr))
        {
            spdlog::error("MGS 2: Thermal Heat - {} shader failed to compile: {}", profile,
                err ? static_cast<const char*>(err->GetBufferPointer()) : "unknown error");
            blob.Reset();
        }
        return blob;
    }

    bool CreateVS(ID3D11Device* device, const char* body, const char* macro, const char* value, HeatVS slot)
    {
        std::string source = body;
        source.replace(source.find("TAIL"), 4, kHeatTail);
        ComPtr<ID3DBlob> vs = CompileHeat(source, macro, value, "vs_5_0");
        return vs && SUCCEEDED(device->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), nullptr, gHeatVS[slot].GetAddressOf()));
    }

    bool CreatePS(ID3D11Device* device, const char* scale, ComPtr<ID3D11PixelShader>& out)
    {
        ComPtr<ID3DBlob> ps = CompileHeat(kHeatPS, "SCALE", scale, "ps_5_0");
        return ps && SUCCEEDED(device->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), nullptr, out.GetAddressOf()));
    }

    bool CreateResources(ID3D11Device* device)
    {
        if (!g_D3D11Hooks.D3DCompileFunc ||
            !CreateVS(device, kHeatSkinnedVS, "UVSET", "0", kSkinnedUv0) ||
            !CreateVS(device, kHeatSkinnedVS, "UVSET", "1", kSkinnedUv1) ||
            !CreateVS(device, kHeatSkinnedVS, "UVSET", "2", kSkinnedUv2) ||
            !CreateVS(device, kHeatRigidVS, "NRMSCALE", "1.0", kRigid) ||
            !CreateVS(device, kHeatRigidVS, "NRMSCALE", "(1.0 / 4096.0)", kRigidShortNrm) ||
            !CreateVS(device, kHeatMorphVS, "NRMSCALE", "1.0", kMorph) ||
            !CreatePS(device, "1.0", gHeatLit) ||
            !CreatePS(device, "(128.0 / 255.0)", gHeatPaint))
        {
            return false;
        }
        D3D11_BLEND_DESC bd = {};
        bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
        return SUCCEEDED(device->CreateBlendState(&bd, gOpaqueBlend.GetAddressOf()));
    }
}

void MGS2ThermalHeat::Initialize()
{
    if (!(eGameType & MGS2) || !bEnabled)
    {
        return;
    }

    uint8_t* status = Memory::PatternScan(baseModule, "F6 05 ?? ?? ?? ?? ?? 0F 84 ?? ?? ?? ?? 41 F6 45",
        "MGS 2: Thermal Heat - system\\libdg\\evmobjs.c -> BP_ChainEvmObj() -> DG_DisplayStatus");
    uint8_t* dispatch = Memory::PatternScan(baseModule, "41 8B 06 FF C8 83 F8",
        "MGS 2: Thermal Heat - bp\\shared\\BP_Renderer.cpp -> BP_RenderCommands()");
    if (!status || !dispatch)
    {
        return;
    }
    pDisplayStatus = reinterpret_cast<const uint32_t*>(status + 7 + *reinterpret_cast<int32_t*>(status + 2));

    // Find the handlers through the command table: Blood Stains hooks some of them first, so a
    // byte pattern would miss.
    uint8_t* base = reinterpret_cast<uint8_t*>(baseModule);
    const int32_t* cases = reinterpret_cast<const int32_t*>(base + *reinterpret_cast<int32_t*>(dispatch + 18));
    auto hook = [&](int cmd, SafetyHookInline& slot, void (__fastcall* fn)(uint8_t*), const char* name)
    {
        uint8_t* stub = base + cases[cmd - 1];   // mov rcx, [r14+8]; call handler
        if (stub[4] != 0xE8) return false;
        slot = safetyhook::create_inline(stub + 9 + *reinterpret_cast<int32_t*>(stub + 5), fn);
        LOG_HOOK(slot, name)
        return static_cast<bool>(slot);
    };
    if (!hook(kCmd_Obj_LocalParam, gLocalParamHook, HookedObjLocalParam, "MGS 2: Thermal Heat - main\\BP_RenderObj.cpp -> BP_Obj_LocalParam()") ||
        !hook(kCmd_Obj_Render, gObjRenderHook, HookedObjRender, "MGS 2: Thermal Heat - main\\BP_RenderObj.cpp -> BP_Obj_Render()") ||
        !hook(kCmd_Obj_RenderMulti, gObjRenderMultiHook, HookedObjRenderMulti, "MGS 2: Thermal Heat - main\\BP_RenderObj.cpp -> BP_Obj_RenderMulti()") ||
        !hook(kCmd_Evm_LocalParam, gEvmLocalParamHook, HookedEvmLocalParam, "MGS 2: Thermal Heat - main\\BP_RenderEvm.cpp -> BP_Evm_LocalParam()") ||
        !hook(kCmd_Evm_Render, gEvmRenderHook, HookedEvmRender, "MGS 2: Thermal Heat - main\\BP_RenderEvm.cpp -> BP_Evm_Render()") ||
        !hook(kCmd_Evm_RenderMulti, gEvmRenderMultiHook, HookedEvmRenderMulti, "MGS 2: Thermal Heat - main\\BP_RenderEvm.cpp -> BP_Evm_RenderMulti()"))
    {
        spdlog::error("MGS 2: Thermal Heat - BP_RenderCommands() case table did not resolve; disabling.");
        gObjRenderHook = {};
    }
}

void MGS2ThermalHeat::OnDeviceReady()
{
    if (!(eGameType & MGS2) || !bEnabled || !gObjRenderHook)
    {
        return;
    }

    auto* device = g_D3D11Hooks.d3dDevice.Get();
    auto* ctx = g_D3D11Hooks.d3dDeviceContext.Get();
    const ULONGLONG started = GetTickCount64();
    if (!device || !ctx || !CreateResources(device))
    {
        spdlog::error("MGS 2: Thermal Heat - no device or resources; disabling.");
        return;
    }
    spdlog::info("MGS 2: Thermal Heat - shaders ready in {} ms.", GetTickCount64() - started);

    void** vtable = *reinterpret_cast<void***>(ctx);
    gDrawIndexedHook = safetyhook::create_inline(vtable[12], reinterpret_cast<void*>(HookedDrawIndexed));
    LOG_HOOK(gDrawIndexedHook, "MGS 2: Thermal Heat - ID3D11DeviceContext::DrawIndexed")
}
