#include <kamek.hpp>
#include <MarioKartWii/Item/ItemBehaviour.hpp>
#include <MarioKartWii/Item/Obj/ObjProperties.hpp>
#include <MarioKartWii/RKNet/RKNetController.hpp>
#include <MarioKartWii/Item/ItemManager.hpp>
#include <MarioKartWii/Kart/KartDamage.hpp>
#include <WTP.hpp>

//WTP Dev Note: Code by Retro Rewind Team

namespace WTP{
namespace Race{

    static void ChangeBlueProp(Item::ObjProperties* dest, const Item::ObjProperties& rel)
    {
        bool itemModeRandom = Pulsar::WTPSETTING_GAMEMODE_REGULAR;
        bool itemModeBlast = Pulsar::WTPSETTING_GAMEMODE_REGULAR;
        bool itemModeShock = Pulsar::WTPSETTING_GAMEMODE_REGULAR;
        if(RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_HOST || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_NONHOST){
            itemModeRandom = System::sInstance->IsContext(Pulsar::PULSAR_GAMEMODERANDOM) ? Pulsar::WTPSETTING_GAMEMODE_RANDOM : Pulsar::WTPSETTING_GAMEMODE_REGULAR;
            itemModeBlast = System::sInstance->IsContext(Pulsar::PULSAR_GAMEMODEBLAST) ? Pulsar::WTPSETTING_GAMEMODE_BLASTBLITZ : Pulsar::WTPSETTING_GAMEMODE_REGULAR;
            itemModeShock = System::sInstance->IsContext(Pulsar::PULSAR_GAMEMODESHOCK) ? Pulsar::WTPSETTING_GAMEMODE_SHOCKTILYOUDROP : Pulsar::WTPSETTING_GAMEMODE_REGULAR;
        }
        new (dest) Item::ObjProperties(rel);
        if(itemModeBlast == Pulsar::WTPSETTING_GAMEMODE_BLASTBLITZ){
            dest->limit = 25;
        }
        if(itemModeRandom == Pulsar::WTPSETTING_GAMEMODE_RANDOM){
            dest->limit = 5;
        }
        if(itemModeShock == Pulsar::WTPSETTING_GAMEMODE_SHOCKTILYOUDROP){
            dest->limit = 25;
        }
    }
    kmCall(0x80790b74, ChangeBlueProp);

    static void ChangeBulletProp(Item::ObjProperties* dest, const Item::ObjProperties& rel)
    {
        bool itemModeRandom = Pulsar::WTPSETTING_GAMEMODE_REGULAR;
        bool itemModeBlast = Pulsar::WTPSETTING_GAMEMODE_REGULAR;
        bool itemModeShock = Pulsar::WTPSETTING_GAMEMODE_REGULAR;
        if(RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_HOST || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_NONHOST){
            itemModeRandom = System::sInstance->IsContext(Pulsar::PULSAR_GAMEMODERANDOM) ? Pulsar::WTPSETTING_GAMEMODE_RANDOM : Pulsar::WTPSETTING_GAMEMODE_REGULAR;
            itemModeBlast = System::sInstance->IsContext(Pulsar::PULSAR_GAMEMODEBLAST) ? Pulsar::WTPSETTING_GAMEMODE_BLASTBLITZ : Pulsar::WTPSETTING_GAMEMODE_REGULAR;
            itemModeShock = System::sInstance->IsContext(Pulsar::PULSAR_GAMEMODESHOCK) ? Pulsar::WTPSETTING_GAMEMODE_SHOCKTILYOUDROP : Pulsar::WTPSETTING_GAMEMODE_REGULAR;
        }
        new (dest) Item::ObjProperties(rel);
        if(itemModeBlast == Pulsar::WTPSETTING_GAMEMODE_BLASTBLITZ){
            dest->limit = 5;
        }
        if(itemModeRandom == Pulsar::WTPSETTING_GAMEMODE_RANDOM){
            dest->limit = 25;
        }
        if(itemModeShock == Pulsar::WTPSETTING_GAMEMODE_SHOCKTILYOUDROP){
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

} // namespace Race   
} // namespace WTP
