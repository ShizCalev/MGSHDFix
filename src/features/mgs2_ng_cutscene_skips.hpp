#pragma once

// MGS2: Stillman's cutscenes and Olga's mid-fight monologues only skip on a repeat playthrough,
// and the staff roll never does. This makes Start skip them all, and a skipped staff roll still
// counts in full on the game clock.
namespace MGS2NGCutsceneSkips
{
    inline bool bStillman = false;
    inline bool bOlgaTaunt = false;
    inline bool bEnding = false;
    void Initialize();
}
