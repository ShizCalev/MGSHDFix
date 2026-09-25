#include "stdafx.h"
#include "mgs2_light_stain.hpp"

#include "common.hpp"
#include "game_funcs.hpp"
#include "logging.hpp"

#include <unordered_set>

// Sparks and electricity light the walls around them. The PS2 lit a throwaway copy of the wall's colours each frame;
// the port writes them into the wall itself and never puts the originals back, so the last glow stays painted on.

namespace
{
    constexpr ptrdiff_t kChanl_ObjQueue = 0x310;
    constexpr ptrdiff_t kQueue_Count    = 0x4;
    constexpr ptrdiff_t kQueue_Objs     = 0x8;
    constexpr ptrdiff_t kObjs_Flag      = 0x58;
    constexpr ptrdiff_t kObjs_RgbBuff   = 0x88;
    constexpr uint32_t  kFlag_Paint     = 0x1;

    SafetyHookInline gTmpLightHook {};
    SafetyHookMid    gComposeLitHook {};

    using ComposeFn = void(__fastcall*)(uint8_t* objs, uint32_t* colors);
    ComposeFn Compose = nullptr;

    std::unordered_set<uint8_t*> gLitNow;
    std::unordered_set<uint8_t*> gStained;

    // DG_TmpLightPreshadeChanl( DG_CHANL *cp, int which )
    void __fastcall TmpLightPreshadeChanl_hooked(uint8_t* chanl, int which)
    {
        gLitNow.clear();
        gTmpLightHook.fastcall<void>(chanl, which);

        uint8_t* queue = *reinterpret_cast<uint8_t**>(chanl + kChanl_ObjQueue);
        if (queue == nullptr)
        {
            return;
        }

        // only objects still in the draw queue are alive to wash
        int count = *reinterpret_cast<int*>(queue + kQueue_Count);
        uint8_t** list = *reinterpret_cast<uint8_t***>(queue + kQueue_Objs);
        for (int i = 0; i < count; ++i)
        {
            uint8_t* objs = list[i];
            if (!gStained.contains(objs) || gLitNow.contains(objs))
            {
                continue;
            }
            gStained.erase(objs);

            uint32_t* rgbs = *reinterpret_cast<uint32_t**>(objs + kObjs_RgbBuff);
            if ((*reinterpret_cast<uint32_t*>(objs + kObjs_Flag) & kFlag_Paint) && rgbs != nullptr)
            {
                Compose(objs, rgbs);
            }
        }

        gStained.insert(gLitNow.begin(), gLitNow.end());
    }
}

void MGS2_LightStain::Setup()
{
    if (!(eGameType & MGS2) || !bEnabled)
    {
        return;
    }

    uint8_t* compose = MGS2_GameFuncs::BP_UpdatePreshadeBuffer;
    uint8_t* tmpLight = Memory::PatternScan(baseModule, "4C 8B DC 48 83 EC ?? 83 3D ?? ?? ?? ?? 00",
        "MGS2: Light Stain: libdg\\pshade.c -> DG_TmpLightPreshadeChanl()");
    uint8_t* composeLit = Memory::PatternScan(baseModule, "E8 ?? ?? ?? ?? 33 C0 48 8B 54 24",
        "MGS2: Light Stain: libdg\\pshade.c -> DG_TmpLightPreshadeChanl() | lit colours upload");
    if (!compose || !tmpLight || !composeLit)
    {
        return;
    }
    Compose = reinterpret_cast<ComposeFn>(compose);

    gComposeLitHook = safetyhook::create_mid(composeLit, [](SafetyHookContext& ctx)
    {
        gLitNow.insert(reinterpret_cast<uint8_t*>(ctx.rcx));
    });
    LOG_HOOK(gComposeLitHook, "MGS2: Light Stain: libdg\\pshade.c -> DG_TmpLightPreshadeChanl() | lit colours upload");

    gTmpLightHook = safetyhook::create_inline(tmpLight, reinterpret_cast<void*>(TmpLightPreshadeChanl_hooked));
    LOG_HOOK(gTmpLightHook, "MGS2: Light Stain: libdg\\pshade.c -> DG_TmpLightPreshadeChanl()");
}
