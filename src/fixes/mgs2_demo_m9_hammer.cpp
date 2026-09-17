#include "stdafx.h"
#include "mgs2_demo_m9_hammer.hpp"

#include "common.hpp"
#include "helper.hpp"
#include "logging.hpp"
#include "mgs2_demo_patches.hpp"

#include "gamevars.hpp"

namespace
{
    constexpr uint32_t kObjectPacket = 4;
    constexpr uint32_t kTermSeq = 0;
    constexpr uint32_t kTermFrame = 1;
    constexpr int32_t  kM92Sub = GameVars::GV_StrCode("m92_sub");
    constexpr int32_t  kFlagJointed = 0x01;      // DG_FLAG_SHADE | DG_FLAG_FINISHCALC, same lighting as one-piece

    // A one-piece object draws every part with the root matrix, so the hammer's pivot offset is lost.
    // The demo already streams a no-op pose for all three joints, so jointed just puts each part at its pivot.
    void Before(uint8_t* stream)
    {
        for (uint8_t* p = stream; p; )
        {
            const uint32_t kind = *reinterpret_cast<uint32_t*>(p) & 0xFFFF;
            const int32_t size = *reinterpret_cast<int32_t*>(p + 4);
            if (kind == kTermSeq || kind == kTermFrame || size < 16)
            {
                break;
            }
            if (kind == kObjectPacket && size >= 32)
            {
                int32_t& flag = *reinterpret_cast<int32_t*>(p + 0x14);
                if ((*reinterpret_cast<int32_t*>(p + 0x1C) & 0xFFFFFF) == kM92Sub && (flag & 3) == 0)
                {
                    flag |= kFlagJointed;
                }
            }
            p += size;
        }
    }
}

void MGS2_DemoM9Hammer::Initialize()
{
    if (!(eGameType & MGS2) || !bEnabled)
    {
        return;
    }

    MGS2_DemoPatches::Add(nullptr, Before, nullptr);    // any cutscene with the M9
}
