#include "stdafx.h"
#include "shutdown_fix.hpp"

#include "common.hpp"
#include "helper.hpp"

void ShutdownFix::Initialize()
{
    if (!(eGameType & (MG | MGS2 | MGS3)))
    {
        return;
    }

    //BP_RenderMain+A2 (MGS2), +142 (MG/MGS3)
    if (uint8_t* pShutdownCall = Memory::PatternScan(baseModule, "E8 ?? ?? ?? ?? 8B CF E8 ?? ?? ?? ?? 85 FF", "MG | MGS 2 | MGS 3: Shutdown Fix: bp\\shared\\BP_Renderer.cpp -> BP_RenderMain(): defer cleanup until game loop exits"))
    {
        // let main() perform its own cleanup at the end of frame, otherwise the game derefs null steam & xaudio handles
        Memory::PatchBytes(reinterpret_cast<uintptr_t>(pShutdownCall), "\x90\x90\x90\x90\x90", 5);
    }
}
