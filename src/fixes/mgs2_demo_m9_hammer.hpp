#pragma once

// MGS2: the tanker cutscenes draw the M9 (m92_sub) as one rigid piece, which ignores each part's pivot, so the
// hammer lands inside Snake's hand. This creates it as a jointed object instead so the hammer sits in place.
namespace MGS2_DemoM9Hammer
{
    inline bool bEnabled = true;
    void Initialize();
}
