#include <core/RK/RKSystem.hpp>
#include <core/nw4r/ut/Misc.hpp>
#include <MarioKartWii/Scene/RootScene.hpp>
#include <MarioKartWii/GlobalFunctions.hpp>
#include <MarioKartWii/RKNet/RKNetController.hpp>
#include <PulsarSystem.hpp>
#include <Extensions/LECODE/LECODEMgr.hpp>
#include <Gamemodes/KO/KOMgr.hpp>
#include <Gamemodes/KO/KOHost.hpp>
#include <Gamemodes/OnlineTT/OnlineTT.hpp>
#include <Settings/Settings.hpp>
#include <Config.hpp>
#include <SlotExpansion/CupsConfig.hpp>
#include <core/egg/DVD/DvdRipper.hpp>

namespace Pulsar {

System* System::sInstance = nullptr;
System::Inherit* System::inherit = nullptr;

void System::CreateSystem() {
    if(sInstance != nullptr) return;
    EGG::Heap* heap = RKSystem::mInstance.EGGSystem;
    const EGG::Heap* prev = heap->BecomeCurrentHeap();
    System* system;
    if(inherit != nullptr) {
        system = inherit->create();
    }
    else system = new System();
    System::sInstance = system;
    ConfigFile& conf = ConfigFile::LoadConfig();
    system->Init(conf);
    prev->BecomeCurrentHeap();
    conf.Destroy();
}
//kmCall(0x80543bb4, System::CreateSystem);
BootHook CreateSystem(System::CreateSystem, 0);

System::System() :
    heap(RKSystem::mInstance.EGGSystem), taskThread(EGG::TaskThread::Create(8, 0, 0x4000, this->heap)),
    //Modes
    koMgr(nullptr), ottHideNames(false) {
}

void System::Init(const ConfigFile& conf) {
    IOType type = IOType_ISO;
    s32 ret = IO::OpenFix("file", IOS::MODE_NONE);

    if(ret >= 0) {
        type = IOType_RIIVO;
        IOS::Close(ret);
    }
    else {
        ret = IO::OpenFix("/dev/dolphin", IOS::MODE_NONE);
        if(ret >= 0) {
            type = IOType_DOLPHIN;
            IOS::Close(ret);
        }
    }
    strncpy(this->modFolderName, conf.header.modFolderName, IOS::ipcMaxFileName);
    static char* pulMagic = reinterpret_cast<char*>(0x800017CC);
    strcpy(pulMagic, "PUL2");

    //InitInstances
    CupsConfig::sInstance = new CupsConfig(conf.GetSection<CupsHolder>());
    this->info.Init(conf.GetSection<InfoHolder>().info);
    this->InitIO(type);
    this->InitSettings(&conf.GetSection<CupsHolder>().trophyCount[0]);


    //Initialize last selected cup and courses
    const PulsarCupId last = Settings::Mgr::sInstance->GetSavedSelectedCup();
    CupsConfig* cupsConfig = CupsConfig::sInstance;
    cupsConfig->SetLayout();
    if(last != -1 && cupsConfig->IsValidCup(last) && cupsConfig->GetTotalCupCount() > 8) {
        cupsConfig->lastSelectedCup = last;
        cupsConfig->SetSelected(cupsConfig->ConvertTrack_PulsarCupToTrack(last, 0));
        cupsConfig->lastSelectedCupButtonIdx = last & 1;
    }

    //Track blocking 
    u32 trackBlocking = this->info.GetTrackBlocking();
    this->netMgr.lastTracks = new PulsarId[trackBlocking];
    for(int i = 0; i < trackBlocking; ++i) this->netMgr.lastTracks[i] = PULSARID_NONE;
    const BMGHeader* const confBMG = &conf.GetSection<PulBMG>().header;
    this->rawBmg = EGG::Heap::alloc<BMGHeader>(confBMG->fileLength, 0x4, RootScene::sInstance->expHeapGroup.heaps[1]);
    memcpy(this->rawBmg, confBMG, confBMG->fileLength);
    this->customBmgs.Init(*this->rawBmg);
    this->AfterInit();
}

//IO
#pragma suppress_warnings on
void System::InitIO(IOType type) const {

    IO* io = IO::CreateInstance(type, this->heap, this->taskThread);
    bool ret;
    if(io->type == IOType_DOLPHIN) ret = ISFS::CreateDir("/shared2/Pulsar", 0, IOS::MODE_READ_WRITE, IOS::MODE_READ_WRITE, IOS::MODE_READ_WRITE);
    const char* modFolder = this->GetModFolder();
    ret = io->CreateFolder(modFolder);
    if(!ret && io->type == IOType_DOLPHIN) {
        char path[0x100];
        snprintf(path, 0x100, "Unable to automatically create a folder for this CT distribution\nPlease create a Pulsar folder in Dolphin Emulator/Wii/shared2", modFolder);
        Debug::FatalError(path);
    }
    char ghostPath[IOS::ipcMaxPath];
    snprintf(ghostPath, IOS::ipcMaxPath, "%s%s", modFolder, "/Ghosts");
    io->CreateFolder(ghostPath);
}
#pragma suppress_warnings reset

void System::InitSettings(const u16* totalTrophyCount) const {
    Settings::Mgr* settings = new (this->heap) Settings::Mgr;
    char path[IOS::ipcMaxPath];
    snprintf(path, IOS::ipcMaxPath, "%s/%s", this->GetModFolder(), "Settings.pul");
    settings->Init(totalTrophyCount, path); //params
    Settings::Mgr::sInstance = settings;
}

void System::UpdateContext() {
    const RacedataSettings& racedataSettings = Racedata::sInstance->menusScenario.settings;
    const GameMode mode = racedataSettings.gamemode;
    this->ottMgr.Reset();
    const Settings::Mgr& settings = Settings::Mgr::Get();
    bool isCT = true;
    bool isHAW = false;
    bool isKO = false;
    bool isOTT = false;
    bool isOTTOnline = settings.GetUserSettingValue(Settings::SETTINGSTYPE_WTP, SETTINGWTP_WWMODE) == WWMODE_OTT && mode == MODE_PUBLIC_VS;
    bool isMiiHeads = settings.GetSettingValue(Settings::SETTINGSTYPE_RACE, SETTINGRACE_RADIO_MII);
    bool is200Online = settings.GetUserSettingValue(Settings::SETTINGSTYPE_WTP, SETTINGWTP_WWMODE) == WWMODE_200 && mode == MODE_PUBLIC_VS;

    const RKNet::Controller* controller = RKNet::Controller::sInstance;
    Network::Mgr& netMgr = this->netMgr;
    const u32 sceneId = GameScene::GetCurrent()->id;


    bool is200 = racedataSettings.engineClass == CC_100 && this->info.Has200cc();
    bool is99999 = settings.GetSettingValue(Settings::SETTINGSTYPE_HOST, HOSTSETTING_CC_99999);
    bool isUltras = settings.GetSettingValue(Settings::SETTINGSTYPE_HOST, SETTINGHOST_ALLOW_ULTRAS) == HOSTSETTING_ULTRAS_ENABLED;
    bool isKOFinal = settings.GetSettingValue(Settings::SETTINGSTYPE_KO, SETTINGKO_FINAL) == KOSETTING_FINAL_ALWAYS;
    bool isOTTChangeCombo = settings.GetSettingValue(Settings::SETTINGSTYPE_OTT, SETTINGOTT_ALLOWCHANGECOMBO) == OTTSETTING_COMBO_ENABLED;
    bool isCharRestrictLight = settings.GetUserSettingValue(Settings::SETTINGSTYPE_WTP, SETTINGWTP_CHARRESTRICT) == WTPSETTING_CHARRESTRICT_LIGHT;
    bool isCharRestrictMedium = settings.GetUserSettingValue(Settings::SETTINGSTYPE_WTP, SETTINGWTP_CHARRESTRICT) == WTPSETTING_CHARRESTRICT_MEDIUM;
    bool isCharRestrictHeavy = settings.GetUserSettingValue(Settings::SETTINGSTYPE_WTP, SETTINGWTP_CHARRESTRICT) == WTPSETTING_CHARRESTRICT_HEAVY;
    bool isKartRestrictKart = settings.GetUserSettingValue(Settings::SETTINGSTYPE_WTP, SETTINGWTP_VEHICLERESTRICT) == WTPSETTING_VEHICLERESTRICT_KARTS;
    bool isKartRestrictBike = settings.GetUserSettingValue(Settings::SETTINGSTYPE_WTP, SETTINGWTP_VEHICLERESTRICT) == WTPSETTING_VEHICLERESTRICT_BIKES;
    bool isItemModeRandom = settings.GetUserSettingValue(Settings::SETTINGSTYPE_WTP, SETTINGWTP_GAMEMODE) == WTPSETTING_GAMEMODE_RANDOM;
    bool isItemModeBlast = settings.GetUserSettingValue(Settings::SETTINGSTYPE_WTP, SETTINGWTP_GAMEMODE) == WTPSETTING_GAMEMODE_BLASTBLITZ;
    bool isItemModeFeather = settings.GetUserSettingValue(Settings::SETTINGSTYPE_WTP, SETTINGWTP_GAMEMODE) == WTPSETTING_GAMEMODE_FEATHERONLY;
    bool isItemModeFeatherless = settings.GetUserSettingValue(Settings::SETTINGSTYPE_WTP, SETTINGWTP_GAMEMODE) == WTPSETTING_GAMEMODE_FEATHERLESS;
    bool isItemModeBobOmb = settings.GetUserSettingValue(Settings::SETTINGSTYPE_WTP, SETTINGWTP_GAMEMODE) == WTPSETTING_GAMEMODE_BOBOMBBLAST;
    bool isItemModeShock = settings.GetUserSettingValue(Settings::SETTINGSTYPE_WTP, SETTINGWTP_GAMEMODE) == WTPSETTING_GAMEMODE_SHOCKTILYOUDROP;
    bool isFeather = this->info.HasFeather();
    bool isUMTs = this->info.HasUMTs();
    bool isMegaTC = this->info.HasMegaTC();
    u32 newContext = 0;
    if(sceneId != SCENE_ID_GLOBE && controller->connectionState != RKNet::CONNECTIONSTATE_SHUTDOWN) {
        switch(controller->roomType) {
            case(RKNet::ROOMTYPE_VS_REGIONAL):
            case(RKNet::ROOMTYPE_JOINING_REGIONAL):
                isOTT = netMgr.ownStatusData == true;
                break;
            case(RKNet::ROOMTYPE_FROOM_HOST):
            case(RKNet::ROOMTYPE_FROOM_NONHOST):
                isCT = mode != MODE_BATTLE && mode != MODE_PUBLIC_BATTLE && mode != MODE_PRIVATE_BATTLE;
                newContext = netMgr.hostContext;
                isKOFinal = newContext & (1 << PULSAR_KOFINAL);
                isCharRestrictLight = newContext & (1 << PULSAR_CHARRESTRICTLIGHT);
                isCharRestrictMedium = newContext & (1 << PULSAR_CHARRESTRICTMEDIUM);
                isCharRestrictHeavy = newContext & (1 << PULSAR_CHARRESTRICTHEAVY);
                isKartRestrictKart = newContext & (1 << PULSAR_KARTRESTRICT);
                isKartRestrictBike = newContext & (1 << PULSAR_BIKERESTRICT);
                isItemModeRandom = newContext & (1 << PULSAR_GAMEMODERANDOM);
                isItemModeBlast = newContext & (1 << PULSAR_GAMEMODEBLAST);
                isItemModeFeather = newContext & (1 << PULSAR_GAMEMODEFEATHER);
                isItemModeFeatherless = newContext & (1 << PULSAR_GAMEMODEFEATHERLESS);
                isItemModeBobOmb = newContext & (1 << PULSAR_GAMEMODEBOBOMB);
                isItemModeShock = newContext & (1 << PULSAR_GAMEMODESHOCK);
                is99999 = newContext & (1 << PULSAR_99999);
                isUltras = newContext & (1 << PULSAR_ULTRAS);
                isHAW = newContext & (1 << PULSAR_HAW);
                isKO = newContext & (1 << PULSAR_MODE_KO);
                isOTT = newContext & (1 << PULSAR_MODE_OTT);
                isOTTOnline |= newContext & (1 << PULSAR_MODE_OTT);
                is200Online |= newContext & (1 << PULSAR_200_WW);
                isMiiHeads = newContext & (1 << PULSAR_MIIHEADS);
                if(isOTT) {
                    isUMTs = newContext & (1 << PULSAR_UMTS);
                    isFeather &= newContext & (1 << PULSAR_FEATHER);
                    isOTTChangeCombo = newContext & (1 << PULSAR_CHANGECOMBO);
                }
                break;
            default: isCT = false;
        }
    }
    else {
        const u8 ottOffline = settings.GetSettingValue(Settings::SETTINGSTYPE_OTT, SETTINGOTT_OFFLINE);
        isOTT = (mode == MODE_GRAND_PRIX || mode == MODE_VS_RACE) ? (ottOffline != OTTSETTING_OFFLINE_DISABLED) : false; //offlineOTT
        if(isOTT) {
            isFeather &= (ottOffline == OTTSETTING_OFFLINE_FEATHER);
            isUMTs &= ~settings.GetSettingValue(Settings::SETTINGSTYPE_OTT, SETTINGOTT_ALLOWUMTS);
            isUltras &= ~settings.GetSettingValue(Settings::SETTINGSTYPE_HOST, SETTINGHOST_ALLOW_ULTRAS);
        }
    }
    this->netMgr.hostContext = newContext;

    // First clear everything except 200_WW and OTTOnline bits
    u32 preserved = this->context & ((1 << PULSAR_200_WW) | (1 << PULSAR_MODE_OTT));

    u32 context = (isCT << PULSAR_CT) | (isHAW << PULSAR_HAW) | (isMiiHeads << PULSAR_MIIHEADS);
    if(isCT) { //contexts that should only exist when CTs are on
        context |= (is200 << PULSAR_200) | (is99999 << PULSAR_99999) | (isFeather << PULSAR_FEATHER) | (isUMTs << PULSAR_UMTS) | 
        (isMegaTC << PULSAR_MEGATC) | (isOTT << PULSAR_MODE_OTT) | (isKO << PULSAR_MODE_KO) | (isUltras << PULSAR_ULTRAS) | (isCharRestrictLight << PULSAR_CHARRESTRICTLIGHT) | 
        (isCharRestrictMedium << PULSAR_CHARRESTRICTMEDIUM) | (isCharRestrictHeavy << PULSAR_CHARRESTRICTHEAVY) | (isKartRestrictKart << PULSAR_KARTRESTRICT) | 
        (isKartRestrictBike << PULSAR_BIKERESTRICT) | (isItemModeRandom << PULSAR_GAMEMODERANDOM) | (isItemModeBlast << PULSAR_GAMEMODEBLAST) | 
        (isItemModeFeather << PULSAR_GAMEMODEFEATHER) | (isItemModeFeatherless << PULSAR_GAMEMODEFEATHERLESS) | 
        (isItemModeBobOmb << PULSAR_GAMEMODEBOBOMB) | (isItemModeShock << PULSAR_GAMEMODESHOCK) | 
        (isKOFinal << PULSAR_KOFINAL) | (isOTTChangeCombo << PULSAR_CHANGECOMBO);
    }
    this->context = context | preserved;

    //Create temp instances if needed:
    /*
    if(sceneId == SCENE_ID_RACE) {
        if(this->lecodeMgr == nullptr) this->lecodeMgr = new (this->heap) LECODE::Mgr;
    }
    else if(this->lecodeMgr != nullptr) {
        delete this->lecodeMgr;
        this->lecodeMgr = nullptr;
    }
    */

    if(isKO) {
        if(sceneId == SCENE_ID_MENU && SectionMgr::sInstance->sectionParams->onlineParams.currentRaceNumber == -1) this->koMgr = new (this->heap) KO::Mgr; //create komgr when loading the select phase of the 1st race of a froom
    }
    if(!isKO && this->koMgr != nullptr || isKO && sceneId == SCENE_ID_GLOBE) {
        delete this->koMgr;
        this->koMgr = nullptr;
    }
}

void System::UpdateContextWrapper() {
    System::sInstance->UpdateContext();
}

static Pulsar::Settings::Hook UpdateContext(System::UpdateContextWrapper);

void System::ClearOttContext()
{
    bool isOTTEnabled = Settings::Mgr::Get().GetSettingValue(Settings::SETTINGSTYPE_OTT, SETTINGOTT_OFFLINE);
    if (!isOTTEnabled) {
        sInstance->context &= ~(1 << PULSAR_MODE_OTT);
    }
}

static Pulsar::Settings::Hook UpdateOTTContext(System::ClearOttContext);

s32 System::OnSceneEnter(Random& random) {
    System* self = System::sInstance;
    self->UpdateContext();
    if(self->IsContext(PULSAR_MODE_OTT)) OTT::AddGhostToVS();
    if(self->IsContext(PULSAR_HAW) && self->IsContext(PULSAR_MODE_KO) && GameScene::GetCurrent()->id == SCENE_ID_RACE && SectionMgr::sInstance->sectionParams->onlineParams.currentRaceNumber > 0) {
        KO::HAWChangeData();
    }
    return random.NextLimited(8);
}
kmCall(0x8051ac40, System::OnSceneEnter);

asmFunc System::GetRaceCount() {
    ASM(
        nofralloc;
    lis r5, sInstance@ha;
    lwz r5, sInstance@l(r5);
    lbz r0, System.netMgr.racesPerGP(r5);
    blr;
        )
}

asmFunc System::GetNonTTGhostPlayersCount() {
    ASM(
        nofralloc;
    lis r12, sInstance@ha;
    lwz r12, sInstance@l(r12);
    lbz r29, System.nonTTGhostPlayersCount(r12);
    blr;
        )
}

//Unlock Everything Without Save (_tZ)
kmWrite32(0x80549974, 0x38600001);

//Skip ESRB page
kmRegionWrite32(0x80604094, 0x4800001c, 'E');

const char System::pulsarString[] = "/Pulsar";
const char System::CommonAssets[] = "/CommonAssets.szs";
const char System::breff[] = "/Effect/Pulsar.breff";
const char System::breft[] = "/Effect/Pulsar.breft";
const char* System::ttModeFolders[] ={ "150", "200", "150F", "200F" };

//WTP Pack ID
kmWrite32(0x800017D0, 0x520);

//WTP Pack Version
kmWrite32(0x800017D4, 42);

}//namespace Pulsar