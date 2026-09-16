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

    // item_box.c GetResources(): the pitch is -(irnd() % N - N/2 + 1024). Both sites do a signed
    // remainder; PS2's irnd() is unsigned, so the remainder is just the low bits.
    uint8_t* item = Memory::PatternScan(baseModule,
        "81 E1 ?? ?? ?? ?? 7D ?? FF C9 81 C9 ?? ?? ?? ?? FF C1 B8",
        "MGS 2: Item Toss Fix - okajima\\etc\\item_box.c -> GetResources() item pitch");
    if (item)
    {
        Memory::PatchBytes(reinterpret_cast<uintptr_t>(item), "\x81\xE1\xFF\x01\x00\x00\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90", 18);   // and ecx, 0x1ff
    }

    uint8_t* tag = Memory::PatternScan(baseModule,
        "81 E1 ?? ?? ?? ?? 7D ?? FF C9 83 C9 ?? FF C1 B8",
        "MGS 2: Item Toss Fix - okajima\\etc\\item_box.c -> GetResources() dog tag pitch");
    if (tag)
    {
        Memory::PatchBytes(reinterpret_cast<uintptr_t>(tag), "\x83\xE1\x7F\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90", 15);   // and ecx, 0x7f
    }
}
