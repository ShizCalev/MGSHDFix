#include "stdafx.h"
#include "mgs2_demo_patches.hpp"

#include "common.hpp"
#include "game_funcs.hpp"
#include "gamevars.hpp"
#include "logging.hpp"

namespace
{
    struct Patch
    {
        const char* stage;
        MGS2_DemoPatches::Fn before;
        MGS2_DemoPatches::Fn after;
    };
    std::vector<Patch> gPatches;

    SafetyHookInline gExecHook {};

    void __fastcall HookedExecDemoStream(void* work, uint8_t* stream, int exec)
    {
        for (const Patch& patch : gPatches)
        {
            if (patch.before && (!patch.stage || g_GameVars.IsStage(patch.stage)))
            {
                patch.before(stream);
            }
        }
        gExecHook.fastcall<void>(work, stream, exec);
        for (const Patch& patch : gPatches)
        {
            if (patch.after && (!patch.stage || g_GameVars.IsStage(patch.stage)))
            {
                patch.after(stream);
            }
        }
    }
}

void MGS2_DemoPatches::Add(const char* stage, Fn before, Fn after)
{
    gPatches.push_back({ stage, before, after });
}

void MGS2_DemoPatches::Initialize()
{
    if (!(eGameType & MGS2) || gPatches.empty() || !MGS2_GameFuncs::DM_ExecDemoStream)
    {
        return;
    }

    gExecHook = safetyhook::create_inline(MGS2_GameFuncs::DM_ExecDemoStream, reinterpret_cast<void*>(HookedExecDemoStream));
    LOG_HOOK(gExecHook, "MGS 2: Demo Patches | demo_pkt.c -> DM_ExecDemoStream()")
    spdlog::info("MGS 2: Demo Patches: {} registered.", gPatches.size());
}
