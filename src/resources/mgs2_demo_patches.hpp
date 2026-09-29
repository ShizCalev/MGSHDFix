#pragma once


struct DemoMotion
{
    uint8_t header[0x10];
    int objectId, motionType, startJoint, nJoints;
    float pos[4];
    int16_t rot[4];
    int pad[2];
    float motion[1][4];     // per joint: rotation, then position
};

// MGS2: lets cutscene fixes edit each frame as it plays, all through one hook.
namespace MGS2_DemoPatches
{
    using Fn = void (*)(uint8_t* stream);

    // `before` runs before the game reads the frame, `after` once it has; either can be null.
    // A null stage means every cutscene.
    void Add(const char* stage, Fn before, Fn after);
    void Initialize();   // after every fix has added theirs

    // 5 ticks per frame
    inline int Frame(const uint8_t* stream) { return *reinterpret_cast<const int*>(stream - 8) / 5; }
}
