#include "stdafx.h"
#include "mgs2_demo_bind_pose_marine.hpp"

#include "common.hpp"
#include "gamevars.hpp"
#include "game_stages.hpp"
#include "helper.hpp"
#include "logging.hpp"
#include "mgs2_demo_patches.hpp"

namespace
{
    // Marines the demo draws before their first real pose. show = we must show them ourselves at the cue.
    struct Early { int id; int cue; bool show; bool shown; };
    Early gEarly[] = {
        { 15, 1870, true,  false },
        { 34, 1032, false, false },     // the cutscene shows this one itself
    };

    void (*DM_SetObjectInvisible)(int id, int invisible) = nullptr;

    void After(uint8_t* stream)
    {
        const int frame = MGS2_DemoPatches::Frame(stream);
        for (Early& e : gEarly)
        {
            if (frame < e.cue)
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
    if (!setInvisible)
    {
        return;
    }

    DM_SetObjectInvisible = reinterpret_cast<void (*)(int, int)>(setInvisible);
    MGS2_DemoPatches::Add(MGS2Stages::D12T3, nullptr, After);     // t12a3d.sdt
}
