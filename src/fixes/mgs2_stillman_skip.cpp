#include "stdafx.h"
#include "mgs2_stillman_skip.hpp"

#include "common.hpp"
#include "game_stages.hpp"
#include "gamevars.hpp"
#include "helper.hpp"
#include "logging.hpp"

namespace
{
    // The script asks "Plant clear count != 0?" before it adds the Start-to-skip actor. These are
    // the bytes of that question; turning the "!=" into ">=" makes the answer always yes.
    constexpr uint8_t kClearedGate[] = { 0x37, 0x11, 0x80, 0x00, 0x04, 0xC1, 0xAC, 0xA0 };
    constexpr size_t kOpIndex = 6;
    constexpr uint8_t kOpGe = 0xB0;
    constexpr size_t kScanLimit = 1 << 20;

    SafetyHookInline gLoadScriptHook {};

    // The game hands us each script it just loaded. Only the Dining Hall's asks the question,
    // so everywhere else is left alone.
    int __fastcall HookedLoadScript(uint8_t* script)
    {
        const int result = gLoadScriptHook.fastcall<int>(script);
        if (!script || !(g_GameVars.IsStage(MGS2Stages::W16A) || g_GameVars.IsStage(MGS2Stages::W16B)))
        {
            return result;
        }

        int opened = 0;
        size_t readable = 0;   // no length comes with the script, so check a page at a time
        for (size_t off = 0; off + sizeof(kClearedGate) <= kScanLimit; ++off)
        {
            if (off + sizeof(kClearedGate) > readable)
            {
                if (!Memory::IsReadable(script + readable, 0x1000))
                {
                    break;
                }
                readable += 0x1000;
            }
            if (script[off] == kClearedGate[0] && memcmp(script + off, kClearedGate, sizeof(kClearedGate)) == 0)
            {
                script[off + kOpIndex] = kOpGe;
                ++opened;
                off += sizeof(kClearedGate) - 1;
            }
        }
        if (opened)
        {
            spdlog::info("MGS 2: Stillman Skip - {} cutscene gate(s) opened in the loaded script.", opened);
        }
        return result;
    }
}

void MGS2StillmanSkip::Initialize()
{
    if (!(eGameType & MGS2) || !bEnabled)
    {
        return;
    }

    uint8_t* load = Memory::PatternScan(baseModule,
        "0F B6 41 ?? 0F B6 51 ?? C1 E2 ?? 0B D0 0F B6 41 ?? C1 E2 ?? 0B D0 0F B6 01 C1 E2 ?? 0B D0",
        "MGS 2: Stillman Skip - system\\libgcl\\command.c -> GCL_LoadScript()");
    if (!load)
    {
        return;
    }
    gLoadScriptHook = safetyhook::create_inline(load, reinterpret_cast<void*>(HookedLoadScript));
    LOG_HOOK(gLoadScriptHook, "MGS 2: Stillman Skip - system\\libgcl\\command.c -> GCL_LoadScript()")
}
