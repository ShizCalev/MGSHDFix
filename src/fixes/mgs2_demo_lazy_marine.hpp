#pragma once

// MGS2: in the tanker deck cutscene (t02a1d) a marine patrolling the stern with a flashlight freezes
// mid-stride at frame 4214 while still on screen. This keeps him walking, looping his own steps.
namespace MGS2_DemoLazyMarine
{
    inline bool bEnabled = true;
    void Initialize();
}
