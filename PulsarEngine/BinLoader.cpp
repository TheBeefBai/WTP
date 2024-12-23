#include <MarioKartWii/Archive/ArchiveMgr.hpp>
#include <MarioKartWii/RKNet/RKNetController.hpp>
#include <WTP.hpp>

//WTP Dev Note: Code by BrawlboxGaming, modified by Retro Rewind Team

namespace WTP{
void *GetCustomKartParam(ArchiveMgr *archive, ArchiveSource type, const char *name, u32 *length){
    if(static_cast<Pulsar::WTPSettingTransmission>(Pulsar::Settings::Mgr::Get().GetUserSettingValue(static_cast<Pulsar::Settings::UserType>(Pulsar::Settings::SETTINGSTYPE_WTP), Pulsar::SETTINGWTP_TRANSMISSION)) == Pulsar::WTPSETTING_TRANSMISSION_DEFAULT){
        name = "kartParam.bin";
    }
    else if(static_cast<Pulsar::WTPSettingTransmission>(Pulsar::Settings::Mgr::Get().GetUserSettingValue(static_cast<Pulsar::Settings::UserType>(Pulsar::Settings::SETTINGSTYPE_WTP), Pulsar::SETTINGWTP_TRANSMISSION)) == Pulsar::WTPSETTING_TRANSMISSION_ALLINSIDE){
        name = "kartParamAll.bin";
    }
    else if(static_cast<Pulsar::WTPSettingTransmission>(Pulsar::Settings::Mgr::Get().GetUserSettingValue(static_cast<Pulsar::Settings::UserType>(Pulsar::Settings::SETTINGSTYPE_WTP), Pulsar::SETTINGWTP_TRANSMISSION)) == Pulsar::WTPSETTING_TRANSMISSION_BIKEINSIDE){
        name = "kartParamBike.bin";
    }
    else if(static_cast<Pulsar::WTPSettingTransmission>(Pulsar::Settings::Mgr::Get().GetUserSettingValue(static_cast<Pulsar::Settings::UserType>(Pulsar::Settings::SETTINGSTYPE_WTP), Pulsar::SETTINGWTP_TRANSMISSION)) == Pulsar::WTPSETTING_TRANSMISSION_ALLOUTSIDE){
        name = "kartParamOut.bin";
    }

    return archive->GetFile(type, name, length);
}
kmCall(0x80591a30, GetCustomKartParam);

void *GetCustomItemSlot(ArchiveMgr *archive, ArchiveSource type, const char *name, u32 *length){
    const RacedataScenario& scenario = Racedata::sInstance->racesScenario;
    const GameMode mode = scenario.settings.gamemode;
    bool itemModeRandom = Pulsar::WTPSETTING_GAMEMODE_REGULAR;
    bool itemModeBlast = Pulsar::WTPSETTING_GAMEMODE_REGULAR;
    bool itemModeFeather = Pulsar::WTPSETTING_GAMEMODE_FEATHERONLY;

    if(RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_HOST || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_NONHOST || mode == MODE_VS_RACE || mode == MODE_BATTLE){
        itemModeRandom = System::sInstance->IsContext(Pulsar::PULSAR_GAMEMODERANDOM) ? Pulsar::WTPSETTING_GAMEMODE_RANDOM : Pulsar::WTPSETTING_GAMEMODE_REGULAR;
        itemModeBlast = System::sInstance->IsContext(Pulsar::PULSAR_GAMEMODEBLAST) ? Pulsar::WTPSETTING_GAMEMODE_BLASTBLITZ : Pulsar::WTPSETTING_GAMEMODE_REGULAR;
        itemModeFeather = System::sInstance->IsContext(Pulsar::PULSAR_GAMEMODEFEATHER) ? Pulsar::WTPSETTING_GAMEMODE_FEATHERONLY : Pulsar::WTPSETTING_GAMEMODE_REGULAR;
    }
    if (itemModeRandom == Pulsar::WTPSETTING_GAMEMODE_REGULAR || itemModeBlast == Pulsar::WTPSETTING_GAMEMODE_REGULAR || itemModeFeather == Pulsar::WTPSETTING_GAMEMODE_REGULAR)
    {
        name="ItemSlotWTP.bin";
    }
    if (itemModeFeather == Pulsar::WTPSETTING_GAMEMODE_FEATHERONLY)
    {
        name="ItemSlotFeather.bin";
    }
    if (itemModeRandom == Pulsar::WTPSETTING_GAMEMODE_RANDOM)
    {
        name="ItemSlotRandom.bin";
    }
    if (itemModeBlast == Pulsar::WTPSETTING_GAMEMODE_BLASTBLITZ)
    {
        name="ItemSlotBlast.bin";
    }
    return archive->GetFile(type, name, length);

}
kmCall(0x807bb128, GetCustomItemSlot);
kmCall(0x807bb030, GetCustomItemSlot);
kmCall(0x807bb200, GetCustomItemSlot);
kmCall(0x807bb53c, GetCustomItemSlot);
kmCall(0x807bbb58, GetCustomItemSlot);
}