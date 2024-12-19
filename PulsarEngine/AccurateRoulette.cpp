#include <kamek.hpp>
#include <MarioKartWii/Item/ItemSlot.hpp>
#include <MarioKartWii/Item/ItemManager.hpp>
#include <MarioKartWii/Race/RaceData.hpp>
#include <MarioKartWii/Race/RaceInfo/RaceInfo.hpp>
#include <SlotExpansion/CupsConfig.hpp>
#include <MarioKartWii/RKNet/RKNetController.hpp>
#include <WTP.hpp>

namespace WTP{
    namespace Race{
        static int AccurateRoulette(Item::ItemSlotData *itemSlotData, u16 itemBoxType, u8 position, ItemId prevRandomItem, bool r7){
            const u8 playerId = Raceinfo::sInstance->playerIdInEachPosition[position - 1];
            Item::Player *itemPlayer = &Item::Manager::sInstance->players[playerId];
            return itemSlotData->DecideItem(itemBoxType, position, itemPlayer->isHuman, 0x1, itemPlayer);
            //return itemSlotData->DecideRouletteItem(itemBoxType, position, prevRandomItem, r7);
        }
        kmCall(0x807ba1e4, AccurateRoulette);
        kmCall(0x807ba428, AccurateRoulette);
        kmCall(0x807ba598, AccurateRoulette);
    }
}