#pragma once

enum MG2PortraitFrame : uint8_t
{
    MG2_PORTRAIT_BIG_BOSS = 0,
    MG2_PORTRAIT_GRAY_FOX = 1,
    MG2_PORTRAIT_GRAY_FOX_DAMAGED = 2,
    MG2_PORTRAIT_HOLLY = 3,
    MG2_PORTRAIT_KASLER = 4,
    MG2_PORTRAIT_MARV = 5,
    MG2_PORTRAIT_CAMPBELL = 6,
    MG2_PORTRAIT_MADNAR = 7,
    MG2_PORTRAIT_MASTER_MILLER = 8,
    MG2_PORTRAIT_GUSTAVA_EYES_OPEN = 9,
    MG2_PORTRAIT_GUSTAVA_EYES_CLOSED = 10,
    MG2_PORTRAIT_JACOBSEN = 11,
    MG2_PORTRAIT_SNAKE = 12,
    MG2_PORTRAIT_SNAKE_SMOKING = 13,
    MG2_PORTRAIT_GUSTAVA_TRANSLATING = 14,
    MG2_PORTRAIT_NONE = 15,
};

enum MG2Scene : uint8_t
{
    MG2_SCENE_GAMEPLAY = 18,
    MG2_SCENE_TRANSCEIVER = 20,
    MG2_SCENE_RESULTS = 25,
};

enum MG2AlertMode : uint8_t
{
    MG2_ALERT_MODE_NORMAL = 0,
    MG2_ALERT_MODE_COUNTDOWN = 1,
    MG2_ALERT_MODE_ALERT = 2,
};

// ScriptCommand command bytes (don't confuse with VM opcodes - they're seperate)
enum MG2ScriptCommand : uint8_t
{
    MG2_COMMAND_MESSAGE_TEXT = 0x06,
    MG2_COMMAND_SCENE = 0x33,
    MG2_COMMAND_SPRITE_SET = 0x66,
};

// Word indices in the ScriptRun interpreter state.
enum MG2ScriptStateWord : int
{
    MG2_SCRIPT_PROG_COUNT = 4,
    MG2_SCRIPT_INSTRUCTION_PROG_COUNT = 7,
};

enum MG2SpriteSheet : int
{
    MG2_SPRITE_SHEET_GUARD = 20,
    MG2_SPRITE_SHEET_PORTRAITS = 25,
    MG2_SPRITE_SHEET_TRANSCEIVER = 38,
};

enum MG2SpriteList : int
{
    MG2_SPRITE_LIST_TRANSCEIVER = 2,
};

// Sprite-list slots assigned by transceiver scripts 111 and 114.
enum MG2TransceiverSlot : int
{
    MG2_TRANSCEIVER_SLOT_BACKGROUND = 0,
    MG2_TRANSCEIVER_SLOT_STATUS = 1,
    MG2_TRANSCEIVER_SLOT_SNAKE = 2,
    MG2_TRANSCEIVER_SLOT_CONTACT = 3,
    MG2_TRANSCEIVER_SLOT_CONNECTION = 5,
};

// Animation and static-frame indices in sprite sheet 38.
enum MG2TransceiverAnimation : int
{
    MG2_TRANSCEIVER_ANIMATION_STATIC = 0,
    MG2_TRANSCEIVER_ANIMATION_CONNECT = 3,
};

enum MG2TransceiverFrame : int
{
    MG2_TRANSCEIVER_FRAME_BACKGROUND = 5,
    MG2_TRANSCEIVER_FRAME_RECEIVE = 6,
    MG2_TRANSCEIVER_FRAME_SEND = 7,
};
