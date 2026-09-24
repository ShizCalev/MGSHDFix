#pragma once

namespace MGS2BloodStains
{
    void Initialize();
    void Init();
    bool InOverlay();   // true while the blood overlay pass is drawing

    inline bool bEnabled = true;
};
