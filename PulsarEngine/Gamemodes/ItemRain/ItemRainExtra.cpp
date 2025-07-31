#include <WTP.hpp>
#include <MarioKartWii/RKNet/RKNetController.hpp>

namespace Pulsar {
namespace ItemRain {

// The following patches are from the original gecko code [Unnamed, MrBean35000vr]
// Increase Item limits
kmWrite8(0x808A5D47, 0x00000022);
kmWrite8(0x808A5A3F, 0x00000022);
kmWrite8(0x808A538F, 0x00000022);
kmWrite8(0x808A56EB, 0x00000019);
kmWrite8(0x808A548B, 0x00000019);

// Allow items like pows and shocks to be seen by other players in an online race.
asmFunc ItemRainOnlineFix() {
    ASM(
        loc_0x0:
            lbz r3, 27(r27);
            cmpwi r3, 0x7;
            beq+ loc_0x24;
            cmpwi r3, 0x3;
            beq+ loc_0x24;
            cmpwi r3, 0x4;
            beq+ loc_0x24;
            addi r3, r4, 0xC8;
            b loc_0x2C;

        loc_0x24:
            mr r3, r4;
            b loc_0x2C;

        loc_0x2C:
    )
}
kmCall(0x8065BB40, ItemRainOnlineFix);

} // namespace ItemRain
} // namespace Pulsar