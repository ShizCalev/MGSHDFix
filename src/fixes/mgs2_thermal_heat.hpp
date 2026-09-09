#pragma once

// MGS2 thermal goggles: draw warm objects as one heat colour like the PS2, and sample
// textures at its detail level.
namespace MGS2ThermalHeat
{
    inline bool bEnabled = true;
    void Initialize();
    void OnDeviceReady();
}
