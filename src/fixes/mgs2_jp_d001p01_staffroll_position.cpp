#include "stdafx.h"
#include "mgs2_jp_d001p01_staffroll_position.hpp"
#include "common.hpp"
#include "logging.hpp"
#include "game_funcs.hpp"
#include "game_stages.hpp"
#include "gamevars.hpp"

namespace
{
    float JpStaffrollPosXFor(uint32_t tri, uint32_t tex, float currentX)
    {
        if (tri != GameVars::GV_StrCode("terop") || strcmp(Shared_Gamefuncs::GM_GetArea(), MGS2Stages::D001P01) != 0)
        {
            return currentX;
        }

        switch (tex)
        {
        // It's "sr##e", not "sr##j". That's why we need this fix.
        case GameVars::GV_StrCode("sr04e_alp_ovl"):
        case GameVars::GV_StrCode("sr05e_alp_ovl"):
        case GameVars::GV_StrCode("sr06e_alp_ovl"):
        case GameVars::GV_StrCode("sr18e_alp_ovl"):
        case GameVars::GV_StrCode("sr09e_alp_ovl"):
        case GameVars::GV_StrCode("sr12e_alp_ovl"):
        case GameVars::GV_StrCode("sr13e_alp_ovl"):
        case GameVars::GV_StrCode("sr14e_alp_ovl"):
            return (currentX == 75.0f) ? 95.0f : currentX;

        case GameVars::GV_StrCode("sr07e_alp_ovl"):
            return (currentX == 40.0f) ? 20.0f : currentX;

        case GameVars::GV_StrCode("sr08e_alp_ovl"):
            return (currentX == -45.0f) ? -35.0f : currentX;

        default:
            return currentX;
        }
    }
}

void MGS2_JpStaffrollPosition::ApplyFix()
{
    if (!(eGameType & MGS2))
    {
        return;
    }

    if (!Util::IsJapanese())
    {
        spdlog::info("MGS 2: JP Staffroll Position Fix: Launched as a non-Japanese version, skipping.");
        return;
    }

    if (!Shared_Gamefuncs::GM_GetArea)
    {
        spdlog::error("MGS 2: JP Staffroll Position Fix: Shared_Gamefuncs::GM_GetArea is null, cannot apply fixes.");
        return;
    }

    MAKE_HOOK_MID(baseModule, "48 8B 43 58 B1 73 48 8B B0 90 00 00 00 E8 ?? ?? ?? ?? 48 85 C0 74 14", "MGS 2: JP Staffroll Position Fix: mgs2x\\source\\user\\shibata\\effect\\2d_sprt.c -> GetResources() | @l191", {
        float& posX = *reinterpret_cast<float*>(ctx.rsp + 0x30);
        const float newX = JpStaffrollPosXFor(static_cast<uint32_t>(ctx.rsi), static_cast<uint32_t>(ctx.rdi), posX);

        if (newX != posX)
        {
            if (g_Logging.bVerboseLogging)
            {
                spdlog::info("MGS 2: JP Staffroll Position Fix: tex={:#010x} pos.x {} -> {}", static_cast<uint32_t>(ctx.rdi), posX, newX);
            }

            posX = newX;
        }
        });
}