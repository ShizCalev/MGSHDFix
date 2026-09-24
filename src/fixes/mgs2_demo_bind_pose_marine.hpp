#pragma once

// MGS2: the tanker "Ocelot's betrayal" cutscene draws two marines before their first pose, so they
// stand stiff on the lower deck in the background. This hides them until their cue.
namespace MGS2_DemoBindPoseMarine
{
    inline bool bEnabled = true;
    void Initialize();
}
