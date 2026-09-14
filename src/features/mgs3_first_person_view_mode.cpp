#include "stdafx.h"
#if defined(MGS3_FPS_DEV)
#include "mgs3_first_person_view_mode.hpp"

#include "common.hpp"
#include "gamevars.hpp"
#include "input_handler.hpp"
#include "logging.hpp"

void MGS3_First_Person_View::Activate()
{
    if (!(eGameType & MGS3) || !bFirst_Person_View_Enabled)
    {
        return;
    }

    int32_t* enableMovement = g_GameVars.gBP_1stPersonCamera_EnableMovement();
    if (enableMovement == nullptr)
    {
        return;
    }

    *enableMovement = bFirst_Person_View_Movement_Enabled_By_Default;
    g_InputHandler.RegisterHotkey(vkToggle_First_Person_View_Movement, "Toggle first person view movement", [enableMovement]()
                                  {
                                      *enableMovement = !*enableMovement;
                                  });
}
#endif

