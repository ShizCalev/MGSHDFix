#pragma once

// MGS2: the two Stillman cutscenes only get a skip button once you've beaten the Plant. The script
// asks "cleared it before?" before adding that button; this makes the answer always yes.
namespace MGS2StillmanSkip
{
    inline bool bEnabled = false;
    void Initialize();
}
