#include "stdafx.h"
#include "mgs2_item_toss_fix.hpp"

#include "common.hpp"
#include "helper.hpp"
#include "logging.hpp"

void MGS2ItemTossFix::Initialize()
{
    if (!(eGameType & MGS2) || !bEnabled)
    {
        return;
    }

    // PS2 irnd() is unsigned, so item_box.c's "irnd()%N" pitch calc is just the low bits; PC's signed remainder widens the toss cone.
    //_GetResources+269
    if (uint8_t* item = Memory::PatternScan(baseModule, "81 E1 ?? ?? ?? ?? 7D ?? FF C9 81 C9 ?? ?? ?? ?? FF C1 B8", "MGS 2: Item Toss Fix: okajima\\etc\\item_box.c -> GetResources() | @l1581: item pitch"))
    {
        Memory::PatchBytes(reinterpret_cast<uintptr_t>(item), "\x81\xE1\xFF\x01\x00\x00\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90", 18);   // and ecx, 0x1ff
    }

    //_GetResources+2A0
    if (uint8_t* tag = Memory::PatternScan(baseModule, "81 E1 ?? ?? ?? ?? 7D ?? FF C9 83 C9 ?? FF C1 B8", "MGS 2: Item Toss Fix: okajima\\etc\\item_box.c -> GetResources() | @l1584: dog tag pitch"))
    {
        Memory::PatchBytes(reinterpret_cast<uintptr_t>(tag), "\x83\xE1\x7F\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90", 15);   // and ecx, 0x7f
    }
}
