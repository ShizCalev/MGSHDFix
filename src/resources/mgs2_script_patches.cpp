#include "stdafx.h"
#include "mgs2_script_patches.hpp"

#include "common.hpp"
#include "gamevars.hpp"
#include "helper.hpp"
#include "logging.hpp"

namespace
{
    struct Patch
    {
        const char* stage;
        std::vector<uint8_t> find;
        size_t at;
        std::vector<uint8_t> put;
    };
    std::vector<Patch> gPatches;
    constexpr size_t kScanLimit = 1 << 20;

    SafetyHookInline gLoadScriptHook {};

    int __fastcall HookedLoadScript(uint8_t* script)
    {
        const int result = gLoadScriptHook.fastcall<int>(script);
        if (!script)
        {
            return result;
        }

        std::vector<const Patch*> wanted;
        size_t longest = 0;
        for (const Patch& patch : gPatches)
        {
            if (g_GameVars.IsStage(patch.stage))
            {
                wanted.push_back(&patch);
                longest = (std::max)(longest, patch.find.size());
            }
        }
        if (wanted.empty())
        {
            return result;
        }

        int patched = 0;
        size_t readable = 0;   // no length given, so check a page at a time
        for (size_t off = 0; off + longest <= kScanLimit; ++off)
        {
            if (off + longest > readable)
            {
                if (!Memory::IsReadable(script + readable, 0x1000))
                {
                    break;
                }
                readable += 0x1000;
            }
            for (const Patch* patch : wanted)
            {
                if (script[off] == patch->find[0] && memcmp(script + off, patch->find.data(), patch->find.size()) == 0)
                {
                    memcpy(script + off + patch->at, patch->put.data(), patch->put.size());
                    ++patched;
                    break;
                }
            }
        }
        if (patched && g_Logging.bVerboseLogging)
        {
            spdlog::info("MGS 2: Script Patches - {} gcx instruction(s) patched in the loaded script.", patched);
        }
        return result;
    }
}

void MGS2_ScriptPatches::Add(const char* stage, std::vector<uint8_t> find, size_t at, std::vector<uint8_t> put)
{
    gPatches.push_back({ stage, std::move(find), at, std::move(put) });
}

void MGS2_ScriptPatches::Initialize()
{
    if (!(eGameType & MGS2) || gPatches.empty())
    {
        return;
    }

    uint8_t* load = Memory::PatternScan(baseModule,
        "0F B6 41 ?? 0F B6 51 ?? C1 E2 ?? 0B D0 0F B6 41 ?? C1 E2 ?? 0B D0 ?? ?? ?? C1 E2 ?? 0B D0",
        "MGS 2: Script Patches - system\\libgcl\\command.c -> GCL_LoadScript()");
    if (!load)
    {
        return;
    }
    gLoadScriptHook = safetyhook::create_inline(load, reinterpret_cast<void*>(HookedLoadScript));
    LOG_HOOK(gLoadScriptHook, "MGS 2: Script Patches - system\\libgcl\\command.c -> GCL_LoadScript()")
}
