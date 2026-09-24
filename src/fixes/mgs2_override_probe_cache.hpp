#pragma once

namespace MGS2_OverrideProbeCache
{
    void Setup();
    void HandleLevelTransition();

    // Rewrites a game-relative path to the override the loader would open; left alone when the cache is off.
    void Resolve(char* path);

    // When the game itself last looked for a file: stage loads, codec faces, anything.
    ULONGLONG LastGameProbe();

    inline bool bEnabled = true;
};
