#include "stdafx.h"
#include "mgs2_ng_cutscene_skips.hpp"

#include "common.hpp"
#include "game_stages.hpp"
#include "helper.hpp"
#include "logging.hpp"
#include "mgs2_linkvarbuf.hpp"
#include "mgs2_script_patches.hpp"

namespace
{
    // The staff roll is two parts: the proposal scene (a stream) and the credits movie. Neither
    // takes a skip button; the hooks below give each one the usual Start / Circle. The game clock
    // runs through both, so a skip pays the clock what was left.
    SafetyHookMid gEndingCheckHook {};
    SafetyHookMid gEndingPreMovieHook {};
    SafetyHookInline gEndingDieHook {};
    uint32_t gPreMovieDone = 0;   // stream time the credits actor waits for
    uint8_t** gStreamActor = nullptr;   // the stream player, mpegstr.c's pCurrentActor
    size_t gStreamCancelMask = 0;   // its "these buttons cancel me" field
    const uint32_t* gPadRelease = nullptr;   // GV_PadDataDirect[0].release
    constexpr uint32_t kPadDemoCancel = 0x840;   // PAD_B | PAD_STA
    constexpr size_t kEndingDmapackFlag = 0x40;
    constexpr size_t kEndingEndFlag = 0x118;
    constexpr size_t kEndingMovieName = 0x120;
    constexpr uint32_t kDmapackInvisibleMenu = 0x100;
    constexpr int kTicksPerFrame = 5;   // the stream clock runs at 300 a second

    int gLastTick = -1;       // proposal: last stream time seen
    int gCreditsFrames = 0;   // credits: frames played so far

    // How many frames the credits movie runs when left alone, measured in game.
    int CreditsLength(const char* movie)
    {
        if (strstr(movie, "staffroll")) return 25954;
        return 0;   // Snake Tales endings: not measured yet, so they stay unskippable
    }

    void PayClock(int frames, const char* what)
    {
        if (frames <= 0)
        {
            return;
        }
        using namespace MGS2_LinkVarBuf;
        GM_PlayTime = GM_PlayTime + frames;
        GM_StagePlayTime = GM_StagePlayTime + frames;
        spdlog::info("MGS 2: NG Cutscene Skips - {} skipped, {} frames added to the game clock.", what, frames);
    }

    // Proposal: let the stream player cancel on the skip buttons; once it is gone (time reads -1)
    // tell the waiting credits actor the time is up.
    void OnEndingPreMovie(SafetyHookContext& ctx)
    {
        const int tick = static_cast<int>(ctx.rax);
        gCreditsFrames = 0;   // the credits have not started yet
        if (tick >= static_cast<int>(gPreMovieDone))
        {
            gLastTick = -1;   // played out on its own
        }
        else if (tick >= 0)
        {
            gLastTick = tick;
            if (*gStreamActor)
            {
                *reinterpret_cast<uint32_t*>(*gStreamActor + gStreamCancelMask) = kPadDemoCancel;
            }
        }
        else if (gLastTick >= 0)
        {
            PayClock((static_cast<int>(gPreMovieDone) - gLastTick) / kTicksPerFrame - 1, "proposal");   // - this frame
            gLastTick = -1;
            ctx.rax = gPreMovieDone;
        }
    }

    // Credits: the actor's own cancel check, work in rbx. Do what it would do on a skip.
    void OnEndingCheck(SafetyHookContext& ctx)
    {
        uint8_t* work = reinterpret_cast<uint8_t*>(ctx.rbx);
        ++gCreditsFrames;
        if (*gPadRelease & kPadDemoCancel)
        {
            const int length = CreditsLength(reinterpret_cast<const char*>(work + kEndingMovieName));
            if (length == 0)
            {
                return;   // a movie we have no length for stays unskippable
            }
            PayClock(length - gCreditsFrames, "credits");
            *reinterpret_cast<uint32_t*>(work + kEndingDmapackFlag) |= kDmapackInvisibleMenu;
            *reinterpret_cast<int*>(work + kEndingEndFlag) = 1;
        }
    }

    void __fastcall HookedEndingDie(uint8_t* work)
    {
        if (g_Logging.bVerboseLogging)
        {
            spdlog::info("MGS 2: NG Cutscene Skips - staff roll '{}' ended after {} credits frames.",
                reinterpret_cast<const char*>(work + kEndingMovieName), gCreditsFrames);
        }
        gLastTick = -1;
        gCreditsFrames = 0;
        gEndingDieHook.fastcall<void>(work);
    }
}

void MGS2NGCutsceneSkips::Initialize()
{
    if (!(eGameType & MGS2))
    {
        return;
    }

    // Each script asks "seen it before?" before it adds the skip button. These make the answer yes.
    if (bStillman)   // "cleared the Plant before?"
    {
        MGS2_ScriptPatches::Add(MGS2Stages::W16A, { 0x37, 0x11, 0x80, 0x00, 0x04, 0xC1, 0xAC, 0xA0 }, 6, { 0xB0 });
        MGS2_ScriptPatches::Add(MGS2Stages::W16B, { 0x37, 0x11, 0x80, 0x00, 0x04, 0xC1, 0xAC, 0xA0 }, 6, { 0xB0 });
    }
    if (bOlgaTaunt)   // "heard this monologue before?"
    {
        MGS2_ScriptPatches::Add(MGS2Stages::W00B, { 0x35, 0x14, 0x04, 0x03, 0x6D, 0xA0 }, 1, { 0xC2, 0xA0 });
        MGS2_ScriptPatches::Add(MGS2Stages::W00B, { 0x35, 0x14, 0x07, 0x0C, 0x40, 0xA0 }, 1, { 0xC2, 0xA0 });
    }
    if (!bEnding)
    {
        return;   // no staff roll hooks, so nothing touches the game clock
    }

    // Olga's skip test names the pad for us: test [GV_PadDataDirect.release], PAD_B | PAD_STA
    uint8_t* pad = Memory::PatternScan(baseModule,
        "F7 05 ?? ?? ?? ?? ?? ?? ?? ?? 74 ?? F7 81",
        "MGS 2: NG Cutscene Skips - morita\\orga\\action\\orga_teaz.h -> ORG_TeaseStopStream()");
    if (!pad)
    {
        return;
    }
    gPadRelease = reinterpret_cast<const uint32_t*>(Memory::GetAbsolute(reinterpret_cast<uintptr_t>(pad) + 2) + 4);   // imm32 follows the disp

    uint8_t* check = Memory::PatternScan(baseModule,
        "83 BB ?? ?? ?? ?? 00 74 ?? E8 ?? ?? ?? ?? 85 05 ?? ?? ?? ?? 74 ?? 81 4B",
        "MGS 2: NG Cutscene Skips - uehara\\pss\\ending.c -> Act()");
    if (!check)
    {
        return;
    }
    gEndingCheckHook = safetyhook::create_mid(check, OnEndingCheck);
    LOG_HOOK(gEndingCheckHook, "MGS 2: NG Cutscene Skips - uehara\\pss\\ending.c -> Act()")

    uint8_t* player = Memory::PatternScan(baseModule,
        "48 8B 05 ?? ?? ?? ?? 48 85 C0 74 ?? 48 8B 88 ?? ?? ?? ?? 48 85 C9 74 ?? 8B 41",
        "MGS 2: NG Cutscene Skips - uehara\\pss\\mpegstr.c -> GetMovieStreamTime()");
    uint8_t* stream = Memory::PatternScan(baseModule,
        "8B 83 ?? ?? ?? ?? 85 05 ?? ?? ?? ?? 0F 85 ?? ?? ?? ?? 39 BB",
        "MGS 2: NG Cutscene Skips - uehara\\pss\\mpegstr.c -> Act() cancel check");
    uint8_t* wait = Memory::PatternScan(baseModule,
        "3D ?? ?? ?? ?? 0F 8C ?? ?? ?? ?? E8",
        "MGS 2: NG Cutscene Skips - uehara\\pss\\ending.c -> Act() pre-movie");
    if (!player || !stream || !wait)
    {
        return;
    }
    gStreamActor = reinterpret_cast<uint8_t**>(Memory::GetAbsolute(reinterpret_cast<uintptr_t>(player) + 3));
    gStreamCancelMask = *reinterpret_cast<const int32_t*>(stream + 2);
    gPreMovieDone = *reinterpret_cast<const uint32_t*>(wait + 1);   // copied first: the hook's jump lands on these bytes
    gEndingPreMovieHook = safetyhook::create_mid(wait, OnEndingPreMovie);
    LOG_HOOK(gEndingPreMovieHook, "MGS 2: NG Cutscene Skips - uehara\\pss\\ending.c -> Act() pre-movie")

    if (uint8_t* die = Memory::PatternScan(baseModule,
        "40 53 48 83 EC ?? F7 41 ?? ?? ?? ?? ?? 48 8B D9 75 ?? 8B 49",
        "MGS 2: NG Cutscene Skips - uehara\\pss\\ending.c -> Die()"))
    {
        gEndingDieHook = safetyhook::create_inline(die, reinterpret_cast<void*>(HookedEndingDie));
        LOG_HOOK(gEndingDieHook, "MGS 2: NG Cutscene Skips - uehara\\pss\\ending.c -> Die()")
    }
}
