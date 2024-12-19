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

        /* namespace HighDataRate
        {
            kmWrite32(0x800017CA, 0x00FF0100);
            kmWrite32(0x80657EA8, 0x28040007);
            kmWrite32(0x80000000, 0x80008000);
        } */
         
    }

    namespace Gameplay
    {
        namespace ShroomGlitchFix
        {
            //Mushroom Glitch Fix [Vabold]
            kmWrite8(0x807BA077, 0x00000000);
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

        /* namespace MegaFOV
        {
            //Mega Mushroom Increased FOV [TheLordScruffy]
            kmWrite16(0x805793AC, 0x00000006);
            kmWrite32(0x809C0000, 0x83A40024);
            kmWrite32(0x801D0000, 0x4182001C);
            kmWrite32(0x80640004, 0x8063000C);
            kmWrite32(0x80630421, 0x4182000C);
            kmWrite32(0x800041F0, 0x901D0120);
            kmWrite8(0x80000000, 0x00000000);
        } */

        /* namespace FixedMiiStats
        {
            //Fixed Mii Stats [B_squo]
            kmWrite16(0x80592163, 0x00000018);
            kmWrite16(0x80592143, 0x000000E8);
        } */
         
    }
}
}