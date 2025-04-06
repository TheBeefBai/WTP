#include <WTP.hpp>
#include <MarioKartWii/RKNet/RKNetController.hpp>
#include <MarioKartWii/UI/Ctrl/UIControl.hpp>
#include <Network/MatchCommand.hpp>
#include <MarioKartWii/UI/Page/Menu/KartSelect.hpp>

//WTP Dev Note: Code by Retro Rewind Team

namespace WTP{
namespace UI{

    u8 RestrictVehicleSelection(){
        SectionMgr::sInstance->sectionParams->kartsDisplayType = 2;
        bool kartRestrict = Pulsar::WTPSETTING_VEHICLERESTRICT_DEFAULT;
        bool bikeRestrict = Pulsar::WTPSETTING_VEHICLERESTRICT_DEFAULT;

        if (RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_HOST || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_NONHOST){
            kartRestrict = System::sInstance->IsContext(Pulsar::PULSAR_KARTRESTRICT) ? Pulsar::WTPSETTING_VEHICLERESTRICT_KARTS : Pulsar::WTPSETTING_VEHICLERESTRICT_DEFAULT;
            bikeRestrict = System::sInstance->IsContext(Pulsar::PULSAR_BIKERESTRICT) ? Pulsar::WTPSETTING_VEHICLERESTRICT_BIKES : Pulsar::WTPSETTING_VEHICLERESTRICT_DEFAULT;
        }

        const RacedataScenario& scenario = Racedata::sInstance->racesScenario;
        const GameMode mode = scenario.settings.gamemode;

        if(kartRestrict == Pulsar::WTPSETTING_VEHICLERESTRICT_KARTS){
            SectionMgr::sInstance->sectionParams->kartsDisplayType = 0;
        }
        if(bikeRestrict == Pulsar::WTPSETTING_VEHICLERESTRICT_BIKES){
            SectionMgr::sInstance->sectionParams->kartsDisplayType = 1;
        }

        return SectionMgr::sInstance->sectionParams->kartsDisplayType;
    }
    kmCall(0x808455a4, RestrictVehicleSelection);
    kmWrite32(0x808455a8, 0x907f06ec);

    bool isKartAccessible(KartId kart, u32 r4){
        bool ret = IsKartUnlocked(kart, r4);
        bool kartRestrict = Pulsar::WTPSETTING_VEHICLERESTRICT_DEFAULT;
        bool bikeRestrict = Pulsar::WTPSETTING_VEHICLERESTRICT_DEFAULT;

        if (RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_HOST || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_NONHOST){
            kartRestrict = System::sInstance->IsContext(Pulsar::PULSAR_KARTRESTRICT) ? Pulsar::WTPSETTING_VEHICLERESTRICT_KARTS : Pulsar::WTPSETTING_VEHICLERESTRICT_DEFAULT;
            bikeRestrict = System::sInstance->IsContext(Pulsar::PULSAR_BIKERESTRICT) ? Pulsar::WTPSETTING_VEHICLERESTRICT_BIKES : Pulsar::WTPSETTING_VEHICLERESTRICT_DEFAULT;
        }

        if((kart < STANDARD_BIKE_S && bikeRestrict == Pulsar::WTPSETTING_VEHICLERESTRICT_BIKES) || (kart >= STANDARD_BIKE_S && kartRestrict == Pulsar::WTPSETTING_VEHICLERESTRICT_KARTS)){
            ret = false;
        }

        return ret;
    }
    kmCall(0x8084a45c, isKartAccessible);
}
}