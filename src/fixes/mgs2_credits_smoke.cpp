#include "stdafx.h"

#include "mgs2_credits_smoke.hpp"
#include "common.hpp"

#include "logging.hpp"

// The smoke inside the tanker credits was tiny and fast: its UVs are 16x too big (PS2 fixed point),
// and it loops every 96 texels when the texture repeats every 128.

namespace
{
    safetyhook::InlineHook gSetSpriteUV;

    constexpr size_t kTexUVWH = 0x60;   // SPR_SPRITE tex.u, v, w, h

    void __fastcall SetSpriteUV_hooked(uint8_t* prim, int u, int v, int w, int h)
    {
        gSetSpriteUV.fastcall<void>(prim, u, v, w, h);

        float* uvwh = reinterpret_cast<float*>(prim + kTexUVWH);
        for (int i = 0; i < 4; ++i)
        {
            uvwh[i] /= 16.0f;
        }
    }
}

void MGS2_CreditsSmoke::Setup()
{
    if (!(eGameType & MGS2) || !bEnabled)
    {
        return;
    }

    uint8_t* const setSpriteUV = Memory::PatternScan(baseModule, "40 53 55 56 57 48 83 EC ?? 48 8B 81",
        "MGS 2: Credits Smoke: shibata\\demo\\telop_trial.c -> SetSpriteUV()");
    if (setSpriteUV == nullptr)
    {
        return;
    }

    gSetSpriteUV = safetyhook::create_inline(reinterpret_cast<void*>(setSpriteUV),
        reinterpret_cast<void*>(SetSpriteUV_hooked));
    LOG_HOOK(gSetSpriteUV, "MGS 2: Credits Smoke: shibata\\demo\\telop_trial.c -> SetSpriteUV()")

    // loop by the texture's full 128 texels, not 96
    MAKE_HOOK_MID(baseModule, "F3 41 0F 58 CA F3 41 0F 11 4F",
        "MGS 2: Credits Smoke: shibata\\demo\\telop_trial.c -> Act() | smoke scroll wrap", {
            ctx.xmm1.f32[0] += 32.0f;
        });
}
