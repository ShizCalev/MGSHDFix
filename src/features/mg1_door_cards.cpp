#include "stdafx.h"
#include "mg1_door_cards.hpp"

#include "common.hpp"
#include "logging.hpp"
#include "game_funcs.hpp"
#include "mg1_linkvarbuf.hpp"

namespace
{
    SafetyHookMid s_DoorCardHook {};
    SafetyHookMid s_EquipmentScreenHook {};
    SafetyHookMid s_ItemIconHook {};

    void DoorCard_hook(SafetyHookContext& ctx)
    {
        const int nLevel = static_cast<int32_t>(ctx.rcx); //the door's card level
        if (nLevel < 1 || nLevel > MG1_ITEM_INDEX_MAX_CARD_LEVEL)
        {
            return;
        }

        if (MG1_LinkVarBuf::HasItem(MG1_LinkVarBuf::GetCardItemIndex(nLevel)))
        {
            ctx.rcx = static_cast<uint32_t>(MG1_LinkVarBuf::GM_EquippedCardNumber);
        }
    }

    void EquipmentScreen_hook(SafetyHookContext&)
    {
        using namespace MG1_LinkVarBuf;

        int32_t* pSlots = GetItemSlots();
        if (!pSlots)
        {
            return;
        }

        bool bSeenCard = false;
        for (size_t i = 1; i < kItemSlotCount;)
        {
            if (pSlots[i] != MG1_ITEM_INDEX_CARD_1)
            {
                ++i;
                continue;
            }
            if (!bSeenCard)
            {
                bSeenCard = true;
                ++i;
                continue;
            }

            for (size_t j = i; j + 1 < kItemSlotCount; ++j)
            {
                pSlots[j] = pSlots[j + 1];
            }
            pSlots[kItemSlotCount - 1] = 0;

            if (GM_RationSlotIndex > static_cast<int32_t>(i))
            {
                GM_RationSlotIndex = GM_RationSlotIndex - 1;
            }
            if (GM_TransmitterSlotIndex > static_cast<int32_t>(i))
            {
                GM_TransmitterSlotIndex = GM_TransmitterSlotIndex - 1;
            }
        }
    }

    void ItemIcon_hook(SafetyHookContext& ctx)
    {
        if (static_cast<int32_t>(ctx.rcx) != MG1_ITEM_INDEX_CARD_1)
        {
            return;
        }
        if (const int nLevel = MG1_LinkVarBuf::GetHighestCardLevel())
        {
            ctx.r9 = static_cast<uint32_t>(nLevel);
        }
    }
}

void MG1_DoorCards::Initialize()
{
    if (!bEnabled)
    {
        return;
    }

    //sub_180003440+16C
    uint8_t* pDoorCheck = Memory::PatternScan(mg1Module, "3B 88", "MG1: Door Cards : HitDoorZone()+0x16C : card level check");
    //sub_180011700
    uint8_t* pEquipmentScreen = Memory::PatternScan(mg1Module, "41 56 48 83 EC ?? 48 89 5C 24", "MG1: Door Cards : EquipmentScreen() : close up duplicate cards");
    //sub_180012980
    uint8_t* pItemIcon = Memory::PatternScan(mg1Module, "48 89 5C 24 10 48 89 6C 24 18 56 57 41 56 48 83 EC 20", "MG1: Door Cards : DrawItemIcon() : highest card level");

    if (!MG1_Gamefuncs::TestFlag || !pDoorCheck || !pEquipmentScreen || !pItemIcon || !MG1_LinkVarBuf::GetGameRecord())
    {
        spdlog::error("MG1: Door Cards : Failed to find door card addresses. Skipping door card hooks.");
    }
    else
    {
        s_DoorCardHook = safetyhook::create_mid(pDoorCheck, DoorCard_hook);
        LOG_HOOK(s_DoorCardHook, "MG1: Door Cards : HitDoorZone()+0x16C : card level check")

        s_EquipmentScreenHook = safetyhook::create_mid(pEquipmentScreen, EquipmentScreen_hook);
        LOG_HOOK(s_EquipmentScreenHook, "MG1: Door Cards : EquipmentScreen() : close up duplicate cards")

        s_ItemIconHook = safetyhook::create_mid(pItemIcon, ItemIcon_hook);
        LOG_HOOK(s_ItemIconHook, "MG1: Door Cards : DrawItemIcon() : highest card level")

        spdlog::info("MG1: Door Cards : Success. Doors will now check for keycards in the player's inventory.");
    }
}
