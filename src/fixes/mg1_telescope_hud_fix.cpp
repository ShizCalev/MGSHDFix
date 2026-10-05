#include "stdafx.h"
#include "mg1_telescope_hud_fix.hpp"
#include "common.hpp"
#include "logging.hpp"

void MG1_TelescopeHudFix::Apply()
{
    //sub_180011C40+CA
    MAKE_HOOK_MID(mg1Module, "85 C0 74 ?? 8B 0D", "MG1: Telescope HUD Fix : DrawStatusBar()+0xCA", {
        ctx.rax = 1; //always clear the status hud
    });
}
