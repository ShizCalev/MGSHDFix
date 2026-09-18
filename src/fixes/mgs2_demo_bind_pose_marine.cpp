#include "stdafx.h"
#include "mgs2_demo_bind_pose_marine.hpp"

#include "common.hpp"
#include "gamevars.hpp"
#include "game_stages.hpp"
#include "helper.hpp"
#include "logging.hpp"

namespace
{
    // Marines the demo draws before their first real pose. show = we must show them ourselves at the cue.
    struct Early { int id; int cueTick; bool show; bool shown; };
    Early gEarly[] = {
        { 15, 1870 * 5, true,  false },
        { 34, 1032 * 5, false, false },     // the demo's own show command lands on this frame
    };

    void (*DM_SetObjectInvisible)(int id, int invisible) = nullptr;

    SafetyHookInline h_ExecDemoStream{};
    void ExecDemoStream_hook(void* work, uint8_t* stream, int exec)
    {
        h_ExecDemoStream.call<void>(work, stream, exec);
        if (!g_GameVars.IsStage(MGS2Stages::D12T3))     // t12a3d.sdt
        {
            return;
        }

        const int tick = *reinterpret_cast<int*>(stream - 8);   // STREAM_TAG.time, 300 Hz
        for (Early& e : gEarly)
        {
            if (tick < e.cueTick)
            {
                DM_SetObjectInvisible(e.id, 1);
                e.shown = false;
            }
            else if (!e.shown)
            {
                if (e.show)
                {
                    DM_SetObjectInvisible(e.id, 0);
                }
                e.shown = true;
            }
        }
    }
}

void MGS2_DemoBindPoseMarine::Initialize()
{
    if (!(eGameType & MGS2) || !bEnabled)
    {
        return;
    }

    uint8_t* setInvisible = Memory::PatternScan(baseModule,
        "48 89 5C 24 ?? 57 48 83 EC ?? 48 63 FA 8B D9",
        "MGS 2: Bind Pose Marine | demo_obj.c -> DM_SetObjectInvisible()");
    uint8_t* exec = Memory::PatternScan(baseModule,
        "48 89 5C 24 ?? 48 89 6C 24 ?? 48 89 74 24 ?? 57 48 83 EC ?? 33 C0 48 8D 2D",
        "MGS 2: Bind Pose Marine | demo_pkt.c -> DM_ExecDemoStream()");
    if (!setInvisible || !exec)
    {
        return;
    }

    DM_SetObjectInvisible = reinterpret_cast<void (*)(int, int)>(setInvisible);
    h_ExecDemoStream = safetyhook::create_inline(reinterpret_cast<void*>(exec), ExecDemoStream_hook);
    LOG_HOOK(h_ExecDemoStream, "MGS 2: Bind Pose Marine | demo_pkt.c -> DM_ExecDemoStream()")
}
