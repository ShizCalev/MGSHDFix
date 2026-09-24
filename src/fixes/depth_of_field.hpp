#pragma once

class DepthOfFieldFixes final
{
public:
    bool bEnabled = true;
    bool bHalfRes = false;   // blur at half the size Full uses
    float fBlurUvMultiplier = 3.0f;

    void Initialize();
    void OnDeviceReady();
    void OnPresent();
    //void HandleLevelTransition() const;
};

inline DepthOfFieldFixes g_DepthOfFieldFixes;
