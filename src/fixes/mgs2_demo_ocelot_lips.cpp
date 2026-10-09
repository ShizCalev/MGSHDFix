#include "stdafx.h"
#include "mgs2_demo_ocelot_lips.hpp"

#include "common.hpp"
#include "gamevars.hpp"
#include "game_stages.hpp"
#include "helper.hpp"
#include "logging.hpp"
#include "mgs2_demo_patches.hpp"


namespace
{

    constexpr int kOcelot = 49;
    constexpr int kJoints = 88;
    constexpr int kMouth[] = { 22, 23, 24, 25, 34, 35, 36, 37, 42 };    // lips and jaw
    constexpr int kRestFirst = 280;     // mouth shut, just before the line
    constexpr int kFirst = 295;
    constexpr int kLast = 421;          // back at rest
    constexpr float kGain = 3.0f;       // matches his other lines

    // "If you wish to" is one still mouth shape, so add four syllables to open it
    constexpr int kBeatFirst = 300;
    constexpr int kBeatLast = 331;
    constexpr float kBeats = 4.0f;
    constexpr float kJawBeat = 7.0f;
    constexpr float kLipBeat = 5.0f;

    struct Joint { float quat[4]; float trans[4]; };
    Joint gRest[std::size(kMouth)];
    Joint gTrue[std::size(kMouth)];     // the game's values, before we made them bigger
    bool gHaveRest = false;
    bool gEnlarged = false;

    DemoMotion* (*DM_GetMotionData)(int objectId) = nullptr;

    Joint* At(DemoMotion* m, int joint)
    {
        return reinterpret_cast<Joint*>(m->motion[joint * 2]);
    }

    void Mul(float out[4], const float a[4], const float b[4])
    {
        const float r[4] = {
            a[3] * b[0] + a[0] * b[3] + a[1] * b[2] - a[2] * b[1],
            a[3] * b[1] - a[0] * b[2] + a[1] * b[3] + a[2] * b[0],
            a[3] * b[2] + a[0] * b[1] - a[1] * b[0] + a[2] * b[3],
            a[3] * b[3] - a[0] * b[0] - a[1] * b[1] - a[2] * b[2],
        };
        memcpy(out, r, sizeof(r));
    }

    // same movement away from rest, just bigger
    void Enlarge(Joint& out, const Joint& rest, const Joint& now, float turnGain, float slideGain)
    {
        const float inv[4] = { -rest.quat[0], -rest.quat[1], -rest.quat[2], rest.quat[3] };
        float d[4];
        Mul(d, inv, now.quat);
        if (d[3] < 0.0f)
        {
            for (float& c : d) c = -c;
        }

        const float s = std::sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
        if (s > 1e-6f)
        {
            const float half = std::atan2(s, d[3]) * turnGain;
            const float k = std::sin(half) / s;
            const float big[4] = { d[0] * k, d[1] * k, d[2] * k, std::cos(half) };
            Mul(out.quat, rest.quat, big);
        }
        for (int c = 0; c < 3; c++)
        {
            out.trans[c] = rest.trans[c] + (now.trans[c] - rest.trans[c]) * slideGain;
        }
    }

    DemoMotion* Ocelot()
    {
        DemoMotion* m = DM_GetMotionData(kOcelot);
        return (m && m->nJoints == kJoints) ? m : nullptr;
    }

    // each frame adds its change on top, so put the game's own values back first
    void Before(uint8_t* stream)
    {
        DemoMotion* m = Ocelot();
        if (m && gEnlarged && MGS2_DemoPatches::Frame(stream) > kFirst)
        {
            for (size_t i = 0; i < std::size(kMouth); i++)
            {
                *At(m, kMouth[i]) = gTrue[i];
            }
        }
        gEnlarged = false;
    }

    void After(uint8_t* stream)
    {
        const int frame = MGS2_DemoPatches::Frame(stream);
        DemoMotion* m = Ocelot();
        if (!m)
        {
            gHaveRest = false;
            return;
        }

        if (frame < kRestFirst)
        {
            gHaveRest = false;
        }
        else if (frame < kFirst)
        {
            for (size_t i = 0; i < std::size(kMouth); i++)
            {
                gRest[i] = *At(m, kMouth[i]);
            }
            gHaveRest = true;
        }
        else if (frame <= kLast && gHaveRest)
        {
            float beat = 0.0f;
            if (frame >= kBeatFirst && frame < kBeatLast)
            {
                const float t = static_cast<float>(frame - kBeatFirst) / (kBeatLast - kBeatFirst);
                beat = 0.5f - 0.5f * std::cos(t * kBeats * 6.2831853f);
            }

            for (size_t i = 0; i < std::size(kMouth); i++)
            {
                Joint* joint = At(m, kMouth[i]);
                gTrue[i] = *joint;
                const bool jaw = kMouth[i] == 42;
                const bool lowerLip = kMouth[i] >= 22 && kMouth[i] <= 25;
                Enlarge(*joint, gRest[i], gTrue[i],
                    kGain + (jaw ? kJawBeat * beat : 0.0f),
                    kGain + (lowerLip ? kLipBeat * beat : 0.0f));
            }
            gEnlarged = true;
        }
    }
}

void MGS2_DemoOcelotLips::Initialize()
{
    if (!(eGameType & MGS2) || !bEnabled)
    {
        return;
    }

    uint8_t* getMotion = Memory::PatternScan(baseModule,
        "40 53 48 83 EC ?? 8B D9 E8 ?? ?? ?? ?? 48 85 C0 74 ?? 48 8D 90",
        "MGS 2: Ocelot Lips | demo_mtn.c -> DM_GetMotionData()");
    if (!getMotion)
    {
        return;
    }

    DM_GetMotionData = reinterpret_cast<DemoMotion* (*)(int)>(getMotion);
    MGS2_DemoPatches::Add(MGS2Stages::D12T3, Before, After);   // t12a3d.sdt
}
