#pragma once

#if defined(MGS3_FPS_DEV)
namespace MGS3_First_Person_View
{
    void Activate();

    inline bool bFirst_Person_View_Enabled = false;
    inline bool bFirst_Person_View_Movement_Enabled_By_Default = true;
    inline int vkToggle_First_Person_View_Movement = 0;
}
#endif
