#include <MarioKartWii/Race/Racedata.hpp>
#include <SlotExpansion/CupsConfig.hpp>
#include <Settings/UI/SettingsPanel.hpp>
#include <Settings/Settings.hpp>
#include <MarioKartWii/RKNet/RKNetController.hpp>
#include <WTP.hpp>

namespace WTP {
    Pulsar::System *System::Create() {
        return new System();
    }

Pulsar::System::Inherit CreateWTP(System::Create);

bool System::Is99999cc() {
    return Racedata::sInstance->racesScenario.settings.engineClass == CC_50;
}

System::WeightClass System::GetWeightClass(const CharacterId id){
    switch (id)
    {
        case BABY_MARIO:
        case BABY_LUIGI:
        case BABY_PEACH:
        case BABY_DAISY:
        case TOAD:
        case TOADETTE:
        case KOOPA_TROOPA:
        case DRY_BONES:
            return LIGHTWEIGHT;
        case MARIO:
        case LUIGI:
        case PEACH:
        case DAISY:
        case YOSHI:
        case BIRDO:
        case DIDDY_KONG:
        case BOWSER_JR:
            return MEDIUMWEIGHT;
        case WARIO:
        case WALUIGI:
        case DONKEY_KONG:
        case BOWSER:
        case KING_BOO:
        case ROSALINA:
        case FUNKY_KONG:
        case DRY_BOWSER:
            return HEAVYWEIGHT;
        default:
            return MIIS;
    }
}

namespace Codes
{
    namespace Online
    {
        namespace RemoveWW
        {
            //Remove Worldwide Button [Chadderz]
            kmWrite16(0x8064B982, 0x00000005);
            kmWrite32(0x8064BA10, 0x60000000);
            kmWrite32(0x8064BA38, 0x60000000);
            kmWrite32(0x8064BA50, 0x60000000);
            kmWrite32(0x8064BA5C, 0x60000000);
            kmWrite16(0x8064BC12, 0x00000001);
            kmWrite16(0x8064BC3E, 0x00000484);
            kmWrite16(0x8064BC4E, 0x000010D7);
            kmWrite16(0x8064BCB6, 0x00000484);
            kmWrite16(0x8064BCC2, 0x000010D7);
        }
        
        namespace InstantVote
        {
            //Instant Voting Roulette Decide [Ro]
            kmWrite32(0x80643BC4, 0x60000000);
            kmWrite32(0x80643C2C, 0x60000000);
        }

        //Allow WFC on Wiimmfi Patched ISOs
        kmWrite32(0x800EE3A0, 0x2C030000);
        kmWrite32(0x800ECAAC, 0x7C7E1B78);
        
    }

    namespace Gameplay
    {
        namespace ShroomGlitchFix
        {
            //Mushroom Glitch Fix [Vabold]
            kmWrite8(0x807BA077, 0x00000000);

        }

        namespace MegaFOV
        {
            //Mega FOV []
            asmFunc GetMegaFOV() {
                ASM(
                    nofralloc;
                loc_0x0:
                  lwz r4, 0(r28);
                  lwz r29, 36(r4);
                  cmpwi r29, 0x0;
                  beq- loc_0x28;
                  lwz r3, 4(r4);
                  lwz r3, 12(r3);
                  rlwinm. r3, r3, 0, 16, 16;
                  beq- loc_0x28;
                  lis r0, 0x41F0;
                  stw r0, 288(r29);
            
                loc_0x28:
                blr;
                )
            }
            kmCall(0x805793AC, GetMegaFOV);
        }
        namespace FPSPatch
        {
            //Force 30 FPS [Vabold]
            kmWrite32(0x80554224, 0x3C808000);
            kmWrite32(0x80554228, 0x88841204);
            kmWrite32(0x8055422C, 0x48000044);

            void FPSPatch() {
            FPSPatchHook = 0x00;
            if (static_cast<Pulsar::MenuSettingChangeFPSMode>(Pulsar::Settings::Mgr::Get().GetSettingValue(static_cast<Pulsar::Settings::Type>(Pulsar::Settings::SETTINGSTYPE_MENU), Pulsar::SETTINGMENU_CHANGEFPS)) == Pulsar::MENUSETTING_FPS_30) {
            FPSPatchHook = 0x00FF0100;
            }
        }
            static PageLoadHook PatchFPS(FPSPatch);
        } // namespace PatchFPS

        namespace AntiLagStart
        {
            //Anti Lag Start [Ro]
            extern "C" void sInstance__8Racedata(void*);
            asmFunc AntiLagStart(){
                ASM(
                nofralloc;
            loc_0x0:
            lwz r12, sInstance__8Racedata@l(r30);
            lwz r12, 0xB70(r12);
            cmpwi r12, 0x7;
            blt- loc_0x14;
            li r3, 0x1;

            loc_0x14:
            cmpwi r3, 0x0;
            blr;
            )
            }
            kmCall(0x80533430, AntiLagStart);
        } // namespace AntiLagStart

        namespace UltraUncut
        {
            asmFunc GetUltraUncut() {
                ASM(
            loc_0x0:
              lis       r12, 0x8000;
              lbz       r12, 0x1203(r12);
              cmpwi     r12, 0;
              beq       end;
              lbz       r3, 0x1C(r29);
              cmplwi    r3, 0x1;
              ble+      loc_0x10;
              mr        r0, r30;
            
            loc_0x10:
              cmplw     r30, r0;
              blr;
            
            end:
              cmplw     r30, r0;
              blr;
                )
            }
            kmCall(0x8053511C, GetUltraUncut);
            
            void UltraUncutPatch() {
              UltraUncutHook = 0x00FF0100;
              const RacedataScenario& scenario = Racedata::sInstance->racesScenario;
              const GameMode mode = scenario.settings.gamemode;

              if(RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_HOST || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_FROOM_NONHOST || RKNet::Controller::sInstance->roomType == RKNet::ROOMTYPE_NONE){
                if (System::sInstance->IsContext(Pulsar::PULSAR_ULTRAS) == Pulsar::HOSTSETTING_ULTRAS_DISABLED) {
                    UltraUncutHook = 0x00;
                   }
              }
              if(mode == MODE_TIME_TRIAL) {
                UltraUncutHook = 0x00;
              }
            }
            static PageLoadHook PatchUltraUncut(UltraUncutPatch);
        } // namespace UltraUncut
        

        namespace MiiStats
        {
            //Fixed Mii Stats
            kmWrite8(0x80592163, 0x00000018);
            kmWrite8(0x80592143, 0x000000E8);
        }
        
        namespace ChainableBulletBill
        {
            kmWrite32(0x807BA5D0, 0x60000000);
        }
        
        namespace PauseEarly
        {
            asmFunc GetPause() {
                ASM(
                    nofralloc;
                loc_0x0:
                cmpwi r0, 0x3;
                crmove 2, 0;
                blr;
                )
            }
            kmCall(0x80856A24, GetPause);
        }
        
        namespace StarDance
        {
            asmFunc GetStar() {
                ASM(
                    nofralloc;
                loc_0x0:
                  lhz r0, 246(r31);
                  lwz r12, 0(r31);
                  lwz r12, 4(r12);
                  lwz r12, 8(r12);
                  andis. r12, r12, 0x8000;
                  beq- loc_0x20;
                  li r0, 0x8;
                  sth r0, 246(r31);
            
                loc_0x20:
                    blr;
                )
            }
            kmCall(0x807CD2DC, GetStar);
        }

        namespace ThwompFreezeFix
        {
            asmFunc GetThwompAntiFreeze() {
                ASM(
                    nofralloc;
                loc_0x0:
                  cmpwi r3, 0x0;
                  beq- loc_0x10;
                  lwz r12, 0(r3);
                  b loc_0x20;
            
                loc_0x10:
                  mflr r12;
                  addi r12, r12, 0x34;
                  mtlr r12;
                  blr;
            
                loc_0x20:
                    blr;
                )
            }
            kmCall(0x80760A88, GetThwompAntiFreeze);
        }

        namespace ItemRespawn
        {
            kmWrite32(0x80573778, 0xD0230020);

            //Item Box Respawn Modifier [Unnamed]
            asmFunc GetItemBoxRespawn() {
                ASM(
            loc_0x0:
            li        r12, 0x1E;
            stw       r12, 0xB8(r27);
            stw       r0, 0xB0(r27);
                )
            }
            kmCall(0x80828EDC, GetItemBoxRespawn);
        }
        

        namespace ItemDamageModifiers
        {
            //Bob-Omb Explosion [CLF78]
            kmWrite32(0x805731CC, 0x38600002);

            //Blue Shell Explosion [CLF78]
            kmWrite32(0x805731B4, 0x38600002);

            //Instant Squash Recovery [Nutmeg]
            kmWrite32(0x8057982C, 0x38000000);
        }         
    }

    namespace Visual
    {
          void PatchVisuals(){
            RemoveBloom = 0x03000000;
            RemoveBackgroundBlur = 0x3f000000;
        
            if (static_cast<Pulsar::MenuSettingBloom>(Pulsar::Settings::Mgr::Get().GetSettingValue(static_cast<Pulsar::Settings::Type>(Pulsar::Settings::SETTINGSTYPE_MENU), Pulsar::SETTINGMENU_BLOOM)) == Pulsar::MENUSETTING_BLOOM_ENABLED){
                RemoveBloom = 0x00;
                RemoveBackgroundBlur = 0x30000000;
            }
        }
        static PageLoadHook VisualHook(PatchVisuals);
    }

    namespace VR
    {
        //Change VR Limit [XeR]
        kmWrite16(0x8052D286, 0x00007530);
        kmWrite16(0x8052D28E, 0x00007530);
        kmWrite16(0x8064F6DA, 0x00007530);
        kmWrite16(0x8064F6E6, 0x00007530);
        kmWrite16(0x8085654E, 0x00007530);
        kmWrite16(0x80856556, 0x00007530);
        kmWrite16(0x8085C23E, 0x00007530);
        kmWrite16(0x8085C246, 0x00007530);
        kmWrite16(0x8064F76A, 0x00007530);
        kmWrite16(0x8064F776, 0x00007530);
        kmWrite16(0x808565BA, 0x00007530);
        kmWrite16(0x808565C2, 0x00007530);
        kmWrite16(0x8085C322, 0x00007530);
        kmWrite16(0x8085C32A, 0x00007530);

        //VR System Changes [MrBean35000vr]
        //Multiply VR difference by 2 [Winner]
        asmFunc GetVRScaleWin() {
            ASM(
        li r5, 2;
        divw r3, r3, r5;
        extsh r3, r3;
            )
        }
        kmCall(0x8052D150, GetVRScaleWin);

        //Cap VR loss from one victorious opponent between 0 and -8.
        asmFunc GetCapVRLoss() {
            ASM(
        lwz       r3, 0x14(r1);
        cmpwi     r3, -8;
        bge       0f;
        li        r3, -8;
        b         1f;
        0:;
        cmpwi     r3, 0;
        ble       1f;
        li        r3, 0;
        1:;
            )
        }
        kmCall(0x8052D260, GetCapVRLoss);

        //Cap VR gain from one defeated opponent between 2 and 12.
        asmFunc GetCapVRGain() {
            ASM(
        lwz       r3, 0x14(r1);
        cmpwi     r3, 2;
        bge       0f;
        li        r3, 2;
        b         1f;
        0:;
        cmpwi     r3, 12;
        ble       1f;
        li        r3, 12;
        1:;
            )
        }
        kmCall(0x8052D1B0, GetCapVRGain);
    }
}
}