#include "stdafx.h"
#include "mgs2_demo_lazy_marine.hpp"

#include "common.hpp"
#include "game_funcs.hpp"
#include "gamevars.hpp"
#include "game_stages.hpp"
#include "helper.hpp"
#include "logging.hpp"

#include <algorithm>
#include <array>
#include <cmath>

namespace
{
    // demo_mtn.c's expanded DEMO_MOTION, one per object; the object actor reads it every frame.
    struct DemoMotion
    {
        uint8_t header[0x10];
        int objectId, motionType, startJoint, nJoints;
        float pos[4];
        int16_t rot[4];
        int pad[2];
        float motion[1][4];     // per joint: quat xyzw, then translation
    };

    constexpr int kMarine = 7;
    constexpr int kJoints = 21;
    constexpr int kCycleFirst = 3923;       // his cleanest full stride: 105 frames
    constexpr int kCycle = 105;
    constexpr int kFade = 15;               // frames recorded before the cycle, to hide its seam
    constexpr int kFreeze = 4214;           // first frozen frame
    constexpr int kEntryPhase = 77;         // cycle frame that best follows his last live pose
    constexpr int kEntryFade = 24;
    constexpr int kHidden = 5082;           // the demo hides him here
    constexpr float kTargetX = -7192.0f;
    constexpr float kTargetZ = -992.0f;
    constexpr float kTurnRate = 0.0087f;    // 0.5 degrees a frame

    struct Pose
    {
        bool valid = false;
        float x = 0.0f, z = 0.0f;
        float joints[kJoints * 2][4]{};
    };

    std::array<Pose, kFade + kCycle> gTape;     // frames kCycleFirst - kFade .. kCycleFirst + kCycle - 1
    Pose gFrozen;
    int gLastFrame = -1;
    float gX = 0.0f, gZ = 0.0f, gYaw = 0.0f;

    DemoMotion* (*DM_GetMotionData)(int objectId) = nullptr;

    void Grab(Pose& pose, const DemoMotion* m)
    {
        pose.valid = true;
        pose.x = m->pos[0];
        pose.z = m->pos[2];
        memcpy(pose.joints, m->motion, sizeof(pose.joints));
    }

    const Pose& Tape(int slot)
    {
        while (slot > 0 && !gTape[slot].valid)      // a dropped frame borrows the one before it
        {
            slot--;
        }
        return gTape[slot];
    }

    void Blend(float out[][4], const float a[][4], const float b[][4], float w)
    {
        for (int j = 0; j < kJoints * 2; j++)
        {
            const bool quat = (j & 1) == 0;
            const float dot = a[j][0] * b[j][0] + a[j][1] * b[j][1] + a[j][2] * b[j][2] + a[j][3] * b[j][3];
            const float sign = (quat && dot < 0.0f) ? -1.0f : 1.0f;
            float len = 0.0f;
            for (int c = 0; c < 4; c++)
            {
                out[j][c] = a[j][c] * (1.0f - w) + b[j][c] * sign * w;
                len += out[j][c] * out[j][c];
            }
            if (quat && len > 0.0f)
            {
                len = std::sqrt(len);
                for (int c = 0; c < 4; c++)
                {
                    out[j][c] /= len;
                }
            }
        }
    }

    float Wrap(float a)
    {
        while (a > 3.14159265f) a -= 6.2831853f;
        while (a < -3.14159265f) a += 6.2831853f;
        return a;
    }

    // One frame of walking: steer towards the target, then take the recorded step, turned to match.
    void Step(int frame)
    {
        const int phase = (frame - kFreeze + kEntryPhase) % kCycle;
        const Pose& from = Tape(kFade + (phase == 0 ? kCycle - 2 : phase - 1));
        const Pose& to = Tape(kFade + (phase == 0 ? kCycle - 1 : phase));
        const Pose& first = Tape(kFade);
        const Pose& last = Tape(kFade + kCycle - 1);

        const float heading = std::atan2(last.x - first.x, last.z - first.z) + gYaw;
        const float bearing = std::atan2(kTargetX - gX, kTargetZ - gZ);
        gYaw += std::clamp(Wrap(bearing - heading), -kTurnRate, kTurnRate);

        const float dx = to.x - from.x, dz = to.z - from.z;
        const float s = std::sin(gYaw), c = std::cos(gYaw);
        gX += dx * c + dz * s;
        gZ += dz * c - dx * s;
    }

    void Walk(DemoMotion* m, int frame)
    {
        if (gLastFrame < kFreeze)
        {
            Grab(gFrozen, m);
            gX = gFrozen.x;
            gZ = gFrozen.z;
            gYaw = 0.0f;
            gLastFrame = kFreeze - 1;
        }

        const float tx = kTargetX - gX, tz = kTargetZ - gZ;
        const bool arrived = tx * tx + tz * tz < 100.0f * 100.0f;
        for (; gLastFrame < frame && !arrived; gLastFrame++)
        {
            Step(gLastFrame + 1);
        }
        gLastFrame = frame;

        const int phase = (frame - kFreeze + kEntryPhase) % kCycle;
        float pose[kJoints * 2][4];
        memcpy(pose, Tape(kFade + phase).joints, sizeof(pose));
        if (phase >= kCycle - kFade)        // slide into the frames that led up to the cycle's start
        {
            const int k = phase - (kCycle - kFade);
            Blend(pose, Tape(kFade + phase).joints, Tape(k).joints, static_cast<float>(k + 1) / (kFade + 1));
        }
        if (frame - kFreeze < kEntryFade)
        {
            Blend(pose, gFrozen.joints, pose, static_cast<float>(frame - kFreeze + 1) / (kEntryFade + 1));
        }

        // Turn the hips: q' = yaw * q, yaw about +Y.
        const float hy = std::sin(gYaw * 0.5f), hw = std::cos(gYaw * 0.5f);
        const float x = pose[0][0], y = pose[0][1], z = pose[0][2], w = pose[0][3];
        pose[0][0] = hw * x + hy * z;
        pose[0][1] = hw * y + hy * w;
        pose[0][2] = hw * z - hy * x;
        pose[0][3] = hw * w - hy * y;

        memcpy(m->motion, pose, sizeof(pose));
        m->pos[0] = gX;
        m->pos[2] = gZ;
    }

    SafetyHookInline h_ExecDemoStream{};
    void ExecDemoStream_hook(void* work, uint8_t* stream, int exec)
    {
        h_ExecDemoStream.call<void>(work, stream, exec);
        if (!g_GameVars.IsStage(MGS2Stages::D01T))      // t02a1d.sdt
        {
            return;
        }

        const int frame = *reinterpret_cast<int*>(stream - 8) / 5;      // STREAM_TAG.time, 300 Hz
        DemoMotion* m = DM_GetMotionData(kMarine);
        if (!m || m->nJoints != kJoints)
        {
            return;
        }

        const int slot = frame - (kCycleFirst - kFade);
        if (frame < kCycleFirst - kFade)
        {
            gLastFrame = -1;
            for (Pose& pose : gTape)
            {
                pose.valid = false;
            }
        }
        else if (slot < static_cast<int>(gTape.size()))
        {
            Grab(gTape[slot], m);
        }
        else if (frame >= kFreeze && frame < kHidden && gTape[0].valid)
        {
            Walk(m, frame);
        }
        else if (frame >= kHidden && gLastFrame >= kFreeze)
        {
            memcpy(m->motion, gFrozen.joints, sizeof(gFrozen.joints));     // the packets are deltas on this buffer
            m->pos[0] = gFrozen.x;
            m->pos[2] = gFrozen.z;
            gLastFrame = -1;
        }
    }
}

void MGS2_DemoLazyMarine::Initialize()
{
    if (!(eGameType & MGS2) || !bEnabled)
    {
        return;
    }

    uint8_t* getMotion = Memory::PatternScan(baseModule,
        "40 53 48 83 EC ?? 8B D9 E8 ?? ?? ?? ?? 48 85 C0 74 ?? 48 8D 90",
        "MGS 2: Lazy Marine | demo_mtn.c -> DM_GetMotionData()");
    uint8_t* exec = MGS2_GameFuncs::DM_ExecDemoStream;
    if (!getMotion || !exec)
    {
        return;
    }

    DM_GetMotionData = reinterpret_cast<DemoMotion* (*)(int)>(getMotion);
    h_ExecDemoStream = safetyhook::create_inline(reinterpret_cast<void*>(exec), ExecDemoStream_hook);
    LOG_HOOK(h_ExecDemoStream, "MGS 2: Lazy Marine | demo_pkt.c -> DM_ExecDemoStream()")
}
