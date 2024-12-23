#include <kamek.hpp>
#include <MarioKartWii/Item/ItemBehaviour.hpp>
#include <MarioKartWii/Item/Obj/ObjProperties.hpp>
#include <MarioKartWii/RKNet/RKNetController.hpp>
#include <MarioKartWii/Item/ItemManager.hpp>
#include <MarioKartWii/Kart/KartDamage.hpp>
#include <MarioKartWii/RKNet/ITEM.hpp>
#include <WTP.hpp>

//WTP Dev Note: Code by Retro Rewind Team

namespace WTP{
namespace Race
{
    static void ChangeBlueProp(Item::ObjProperties* dest, const Item::ObjProperties& rel)
    {
        bool itemModeRandom = Pulsar::WTPSETTING_GAMEMODE_REGULAR;
        bool itemModeBlast = Pulsar::WTPSETTING_GAMEMODE_REGULAR;
        if(RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_HOST || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_NONHOST){
            itemModeRandom = System::sInstance->IsContext(Pulsar::PULSAR_GAMEMODERANDOM) ? Pulsar::WTPSETTING_GAMEMODE_RANDOM : Pulsar::WTPSETTING_GAMEMODE_REGULAR;
            itemModeBlast = System::sInstance->IsContext(Pulsar::PULSAR_GAMEMODEBLAST) ? Pulsar::WTPSETTING_GAMEMODE_BLASTBLITZ : Pulsar::WTPSETTING_GAMEMODE_REGULAR;
        }
        new (dest) Item::ObjProperties(rel);
        if(itemModeBlast == Pulsar::WTPSETTING_GAMEMODE_BLASTBLITZ){
            dest->limit = 25;
        }
        if(itemModeRandom == Pulsar::WTPSETTING_GAMEMODE_RANDOM){
            dest->limit = 5;
        }
    }
    kmCall(0x80790b74, ChangeBlueProp);

    static void ChangeBulletProp(Item::ObjProperties* dest, const Item::ObjProperties& rel)
    {
        bool itemModeRandom = Pulsar::WTPSETTING_GAMEMODE_REGULAR;
        bool itemModeBlast = Pulsar::WTPSETTING_GAMEMODE_REGULAR;
        if(RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_HOST || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_NONHOST){
            itemModeRandom = System::sInstance->IsContext(Pulsar::PULSAR_GAMEMODERANDOM) ? Pulsar::WTPSETTING_GAMEMODE_RANDOM : Pulsar::WTPSETTING_GAMEMODE_REGULAR;
            itemModeBlast = System::sInstance->IsContext(Pulsar::PULSAR_GAMEMODEBLAST) ? Pulsar::WTPSETTING_GAMEMODE_BLASTBLITZ : Pulsar::WTPSETTING_GAMEMODE_REGULAR;
        }
        new (dest) Item::ObjProperties(rel);
        if(itemModeBlast == Pulsar::WTPSETTING_GAMEMODE_BLASTBLITZ){
            dest->limit = 5;
        }
        if(itemModeRandom == Pulsar::WTPSETTING_GAMEMODE_RANDOM){
            dest->limit = 25;
        }
    }
    kmCall(0x80790bf4, ChangeBulletProp);

    static void ChangeBombProp(Item::ObjProperties* dest, const Item::ObjProperties& rel)
    {
        bool itemModeRandom = Pulsar::WTPSETTING_GAMEMODE_REGULAR;
        bool itemModeBlast = Pulsar::WTPSETTING_GAMEMODE_REGULAR;
        if(RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_HOST || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_NONHOST){
            itemModeRandom = System::sInstance->IsContext(Pulsar::PULSAR_GAMEMODERANDOM) ? Pulsar::WTPSETTING_GAMEMODE_RANDOM : Pulsar::WTPSETTING_GAMEMODE_REGULAR;
            itemModeBlast = System::sInstance->IsContext(Pulsar::PULSAR_GAMEMODEBLAST) ? Pulsar::WTPSETTING_GAMEMODE_BLASTBLITZ : Pulsar::WTPSETTING_GAMEMODE_REGULAR;
        }
        new (dest) Item::ObjProperties(rel);
        if(itemModeBlast == Pulsar::WTPSETTING_GAMEMODE_BLASTBLITZ){
            dest->limit = 25;
        }
        if(itemModeRandom == Pulsar::WTPSETTING_GAMEMODE_RANDOM){
            dest->limit = 20;
        }
    }
    kmCall(0x80790bb4, ChangeBombProp);

    /* static void TripleFeatherFeatherOnly(RKNet::ITEMHandler& itemHandler, Item::PlayerInventory& inventory, ItemId id, u8 playerId, bool isItemForcedDueToCapacity){
        Random random;
        int rnd;
        bool itemModeFeather = Pulsar::WTPSETTING_GAMEMODE_REGULAR;
        const RKNet::Controller* controller = RKNet::Controller::sInstance;
        const RacedataSettings& racedataSettings = Racedata::sInstance->menusScenario.settings;
        const GameMode mode = racedataSettings.gamemode;
        if(RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_HOST || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_NONHOST || mode == MODE_VS_RACE){
            itemModeFeather = System::sInstance->IsContext(Pulsar::PULSAR_GAMEMODEFEATHER) ? Pulsar::WTPSETTING_GAMEMODE_FEATHERONLY : Pulsar::WTPSETTING_GAMEMODE_REGULAR;
        }
        
        if(itemModeFeather == Pulsar::WTPSETTING_GAMEMODE_FEATHERONLY){
            const ItemId item = itemHandler.GetStoredItem(playerId);
            if(item == BLOOPER){
                rnd = random.NextLimited(100);
                if(rnd <= 3){
                    inventory.currentItemCount = 3;
                }
            }
        }
    } */

} // namespace Race   
} // namespace WTP
