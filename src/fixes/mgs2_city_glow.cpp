#include "stdafx.h"

#include "mgs2_city_glow.hpp"
#include "common.hpp"
#include "gamevars.hpp"

#include "logging.hpp"

// Sons of Liberty lit the gaps between the Manhattan buildings with a soft city glow. Substance made it
// its own model at 10x the size, so it vanished past the far plane, and this port lost it. Put it back.

namespace
{
    safetyhook::InlineHook gPutObject;

    // putobj.c reads its settings off the script line, so hand it one: -f 0x10 -m d00a3 -l d00a -s 10000.
    // 10000 is the riverbank's own scale; Substance's 100000 was the bug. 0x5N = option, 0x09 = int.
    constexpr uint8_t kLine[] = {
        0x56, 'f', 0x09, 0x10, 0x00, 0x00, 0x00,
        0x56, 'm', 0x09, 0x59, 0xCC, 0x58, 0x00,
        0x56, 'l', 0x09, 0x61, 0xC6, 0x32, 0x00,
        0x5D, 0x10, 's', 0x09, 0x10, 0x27, 0x00, 0x00,
                         0x09, 0x10, 0x27, 0x00, 0x00,
                         0x09, 0x10, 0x27, 0x00, 0x00,
        0x00,
    };

    constexpr int kRiverbank = 0x0063CF5F;   // 川岸, the riverbank
    constexpr int kName      = 0x00F8A46F;   // 川岸かき割, its backdrop

    void SpawnBackdrop(int where)
    {
        static char* frame[2] = {};
        char** const savedLine = *g_GameVars.GCL_CommandLine();
        char* const savedNext = *g_GameVars.GCL_NextStrPtr();

        frame[0] = reinterpret_cast<char*>(const_cast<uint8_t*>(kLine));
        *g_GameVars.GCL_CommandLine() = &frame[1];
        void* const work = gPutObject.fastcall<void*>(kName, where);
        *g_GameVars.GCL_CommandLine() = savedLine;
        *g_GameVars.GCL_NextStrPtr() = savedNext;

        if (work == nullptr)
        {
            spdlog::warn("MGS 2: City Glow: The backdrop model is not loaded on this stage - skipped.");
        }
        else
        {
            spdlog::info("MGS 2: City Glow: Manhattan backdrop restored.");
        }
    }

    // Spawn it just before the riverbank so the skyline draws over it, and again each time the stage rebuilds it.
    void* __fastcall PutObject_hooked(int name, int where)
    {
        if (name == kRiverbank && g_GameVars.IsStage(MGS2Stages::D00T))
        {
            SpawnBackdrop(where);
        }

        return gPutObject.fastcall<void*>(name, where);
    }
}

void MGS2_CityGlow::Setup()
{
    if (!(eGameType & MGS2) || !bEnabled)
    {
        return;
    }

    if (g_GameVars.GCL_CommandLine() == nullptr || g_GameVars.GCL_NextStrPtr() == nullptr)
    {
        spdlog::info("MGS 2: City Glow: GCL globals not resolved, skipping.");
        return;
    }

    uint8_t* const ctor = Memory::PatternScan(baseModule,
        "48 89 5C 24 ?? 48 89 74 24 ?? 57 48 83 EC ?? 45 33 C9 8B FA 8B F1 BA ?? ?? ?? ?? 41 B8 10 02 00 00",
        "MGS 2: City Glow: korekado\\conv\\putobj.c -> NewPutObject()");
    if (ctor == nullptr)
    {
        return;
    }

    gPutObject = safetyhook::create_inline(reinterpret_cast<void*>(ctor),
        reinterpret_cast<void*>(PutObject_hooked));
    LOG_HOOK(gPutObject, "MGS 2: City Glow: korekado\\conv\\putobj.c -> NewPutObject()")
}
