#pragma once

class DepthOfFieldFixes final
{
public:
    bool bEnabled = true;
    bool bHalfRes = false;   // gather, CoC and pyramid root at quarter res instead of half
    float fBlurUvMultiplier = 3.0f;

    void Initialize();
    void OnDeviceReady();
    void OnPresent();
    //void HandleLevelTransition() const;
};

inline DepthOfFieldFixes g_DepthOfFieldFixes;
