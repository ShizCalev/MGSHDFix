#include "stdafx.h"
#include "mgs2_fast_doors.hpp"

#include "common.hpp"
#include "logging.hpp"

#include <atomic>

// A door fades the music out over a second, and the next area can't load its sounds until it stops. The PS2's disc
// load hid that; on PC the black screen just waits. Fade it in an eighth of a second and skip the name on the black.

namespace
{
    SafetyHookInline gExecProcHook {};
    SafetyHookInline gSetSoundHook {};
    SafetyHookInline gFadeOutHook {};

    constexpr int      kFadeThenLoadProc = 0x23ADF5;   // フェード後ロードへ
    constexpr int      kExitNameProc     = 0xFCC711;   // ステージ名表示（終了時）
    constexpr uint32_t kFadeOutStop1s    = 0x01FFFF0B;   // SNG_FOUTS_S
    constexpr uint32_t kFadeOutStopHalf  = 0x01FFFF0A;   // SNG_FOUTS_SS

    constexpr int       kHalfSecondStep = 0x28F;   // SNG_VOL / (INT1SEC/2), per tick
    constexpr int       kStepScale      = 4;
    constexpr int       kSongTracks     = 32;

    constexpr ULONGLONG kDoorWindowMs = 1000;

    // the fade comes from the door proc on some doors, from the stage teardown a tick later on others
    ULONGLONG gDoorAt = 0;
    std::atomic<ULONGLONG> gSpeedUpUntil { 0 };
    int* gFadeSteps = nullptr;   // sng_fade_time[]

    // void *GCL_ExecProc( int proc_id, GCL_ARGS *args )
    uintptr_t __fastcall ExecProc_hooked(int procId, void* args)
    {
        if (procId == kExitNameProc)
        {
            return 0;
        }
        if (procId != kFadeThenLoadProc)
        {
            return gExecProcHook.fastcall<uintptr_t>(procId, args);
        }
        gDoorAt = GetTickCount64();
        return gExecProcHook.fastcall<uintptr_t>(procId, args);
    }

    // void sd_set_cli( int code )
    void __fastcall SetSound_hooked(uint32_t code)
    {
        if (code == kFadeOutStop1s && gDoorAt && GetTickCount64() - gDoorAt < kDoorWindowMs)
        {
            gDoorAt = 0;
            static bool logged = false;
            if (!logged)
            {
                logged = true;
                spdlog::info("MGS2: Fast Doors: door music fade shortened");
            }
            code = kFadeOutStopHalf;
            gSpeedUpUntil = GetTickCount64() + kDoorWindowMs;
        }
        gSetSoundHook.fastcall<void>(code);
    }

    // sd_drv.c -> SngFadeOutS( unsigned long kind ), a tick later from the sound queue
    int __fastcall FadeOutS_hooked(uint32_t kind)
    {
        const int result = gFadeOutHook.fastcall<int>(kind);
        if (kind == kFadeOutStopHalf && GetTickCount64() < gSpeedUpUntil.exchange(0))
        {
            for (int i = 0; i < kSongTracks; ++i)
            {
                if (gFadeSteps[i] == kHalfSecondStep)
                {
                    gFadeSteps[i] *= kStepScale;
                }
            }
        }
        return result;
    }
}

void MGS2_FastDoors::Setup()
{
    if (!(eGameType & MGS2) || !bEnabled)
    {
        return;
    }

    uint8_t* execProc = Memory::PatternScan(baseModule, "48 89 5C 24 ?? 48 89 6C 24 ?? 48 89 74 24 ?? 57 48 83 EC ?? 44 8B 1D",
        "MGS2: Fast Doors: libgcl -> GCL_ExecProc()");
    uint8_t* setSound = Memory::PatternScan(baseModule, "40 53 48 83 EC ?? 81 0D",
        "MGS2: Fast Doors: sd_ee.c -> sd_set_cli()");
    uint8_t* fadeOut = Memory::PatternScan(baseModule, "48 83 EC ?? 83 3D ?? ?? ?? ?? 00 0F 84 ?? ?? ?? ?? 48 8B 05",
        "MGS2: Fast Doors: sd_drv.c -> SngFadeOutS()");
    uint8_t* fadeSteps = Memory::PatternScan(baseModule, "4C 8D 05 ?? ?? ?? ?? ?? ?? ?? 41 8D 4A",
        "MGS2: Fast Doors: sd_drv.c -> SngFadeOutS() | sng_fade_time");
    if (!execProc || !setSound || !fadeOut || !fadeSteps)
    {
        return;
    }

    // the loop's pointer starts two tracks in
    gFadeSteps = reinterpret_cast<int*>(Memory::GetRipRelativeAddress(fadeSteps, 3, 7)) - 2;

    gFadeOutHook = safetyhook::create_inline(fadeOut, reinterpret_cast<void*>(FadeOutS_hooked));
    LOG_HOOK(gFadeOutHook, "MGS2: Fast Doors: sd_drv.c -> SngFadeOutS()");
    gSetSoundHook = safetyhook::create_inline(setSound, reinterpret_cast<void*>(SetSound_hooked));
    LOG_HOOK(gSetSoundHook, "MGS2: Fast Doors: sd_ee.c -> sd_set_cli()");
    gExecProcHook = safetyhook::create_inline(execProc, reinterpret_cast<void*>(ExecProc_hooked));
    LOG_HOOK(gExecProcHook, "MGS2: Fast Doors: libgcl -> GCL_ExecProc()");
}
