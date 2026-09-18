#include "stdafx.h"
#include "mgs2_solidus_pipe.hpp"

#include "common.hpp"
#include "gamevars.hpp"
#include "helper.hpp"
#include "logging.hpp"

namespace
{
    // w25a_brk02b, the dented pipe: its model header carries this centre and three units.
    constexpr int kModels = 3;
    constexpr float kCentreY = -3499.0f;
    constexpr float kCentreZ = -152500.0f;

    // The dent's vertices in the model's own order, and how much light each keeps (floor darkest).
    struct DentVertex { int index; float keep; };
    constexpr DentVertex kDent[] = {
        { 28, 0.60f }, { 29, 0.60f }, { 32, 0.80f }, { 33, 0.80f }, { 63, 0.80f }, { 65, 0.80f },
        { 67, 0.60f }, { 99, 0.60f }, { 103, 0.60f }, { 105, 0.80f }, { 111, 0.60f }, { 112, 0.60f },
        { 134, 0.80f }, { 136, 0.60f }, { 138, 0.80f }, { 140, 0.80f }, { 143, 0.60f }, { 145, 0.60f },
    };

    uint32_t gShaded[std::size(kDent)] = {};   // what we last wrote, so a re-run never darkens twice

    bool IsDentedPipe(uintptr_t objs)
    {
        const uintptr_t def = Memory::ReadField<uintptr_t>(objs, 0x48);
        return def != 0
            && Memory::ReadField<int>(def, 0x04) == kModels
            && Memory::ReadField<float>(def, 0x38) == kCentreY
            && Memory::ReadField<float>(def, 0x3C) == kCentreZ;
    }

    // The colours are one RGBA byte each, straight from the preshade.
    uint32_t Shade(uint32_t rgba, float keep)
    {
        uint32_t out = rgba & 0xFF000000u;
        for (int shift = 0; shift < 24; shift += 8)
        {
            const uint32_t c = static_cast<uint32_t>(((rgba >> shift) & 0xFF) * keep);
            out |= c << shift;
        }
        return out;
    }

    SafetyHookInline h_Compose{};
    void __fastcall Compose_hook(uintptr_t objs, uint32_t* colors)
    {
        if (colors && g_GameVars.IsAnyStage({ MGS2Stages::A25A, MGS2Stages::W25A, MGS2Stages::D045P01, MGS2Stages::D046P01 })
            && IsDentedPipe(objs))
        {
            for (size_t i = 0; i < std::size(kDent); i++)
            {
                uint32_t& c = colors[kDent[i].index];
                if (c != gShaded[i])
                {
                    c = gShaded[i] = Shade(c, kDent[i].keep);
                }
            }
        }
        h_Compose.fastcall<void>(objs, colors);
    }
}

void MGS2_SolidusPipe::Initialize()
{
    if (!(eGameType & MGS2) || !bEnabled)
    {
        return;
    }

    uint8_t* compose = Memory::PatternScan(baseModule,
        "40 57 48 83 EC ?? 48 0F BF 41",
        "MGS 2: Solidus Pipe | system\\libdg\\pshade.c -> preshade colour composer");
    if (compose)
    {
        h_Compose = safetyhook::create_inline(reinterpret_cast<void*>(compose), Compose_hook);
        LOG_HOOK(h_Compose, "MGS 2: Solidus Pipe | system\\libdg\\pshade.c -> preshade colour composer")
    }
}
