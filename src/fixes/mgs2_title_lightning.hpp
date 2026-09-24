#pragma once

namespace MGS2_TitleLightning
{
    void Setup();
    void Init();
    void HandleLevelTransition();

    inline bool bEnabled = true;
    inline bool bLoaded = false;
};
