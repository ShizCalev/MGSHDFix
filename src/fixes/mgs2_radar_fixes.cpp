#include "stdafx.h"
#include "mgs2_radar_fixes.hpp"

#include "common.hpp"
#include "helper.hpp"
#include "logging.hpp"

void MGS2_RadarFixes::Initialize()
{
    if (!(eGameType & MGS2))
    {
        return;
    }

    // Radar off should still show the ALERT / CAUTION box.
    if (bShowAlertWithRadarOff)
    {
        uint8_t* show = Memory::PatternScan(baseModule,
            "0F 94 C1 8B D9",
            "MGS 2: Radar Fixes | kira\\radar\\radar.c -> CheckRadarPartDisplayFlag()");
        if (show)
        {
            Memory::PatchBytes(reinterpret_cast<uintptr_t>(show), "\xB1\x01\x90", 3);   // mov cl, 1
            spdlog::info("MGS 2: Radar Fixes | kira\\radar\\radar.c -> CheckRadarPartDisplayFlag(): Alert box now shows with the radar off.");
        }
    }

    // Loading a save should keep the save's radar type.
    if (bKeepSavedRadarType)
    {
        constexpr uint32_t kFromSave = 0x3800;          // options a save keeps
        constexpr uint32_t kRadarBits = 0x0004 | 0x0020;    // radar off, type 2
        const std::vector<uint8_t*> loads = Memory::FindMultiplePatternMatches(baseModule,
            "BA ?? ?? ?? ?? 66 33 41 ?? 66 23 C2 66 41 33 C6");
        if (loads.size() != 2)
        {
            spdlog::error("MGS 2: Radar Fixes | kano\\mcman\\mcdata.c -> ComStoreLoadedVariable(): Pattern scan failed, found {} of 2 spots.", loads.size());
            return;
        }
        for (uint8_t* load : loads)
        {
            if (*reinterpret_cast<uint32_t*>(load + 1) == kFromSave)
            {
                Memory::Write(reinterpret_cast<uintptr_t>(load + 1), kFromSave | kRadarBits);
            }
        }
        spdlog::info("MGS 2: Radar Fixes | kano\\mcman\\mcdata.c -> ComStoreLoadedVariable(): Saves now keep their radar type.");
    }
}
