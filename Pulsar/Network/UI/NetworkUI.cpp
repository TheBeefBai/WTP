#include <kamek.hpp>
#include <game/UI/Page/Other/GlobeSearch.hpp>
#include <PulsarSystem.hpp>
#include <UI/UI.hpp>

kmWrite32(0x80609268, 0x7f63db78);
namespace Pulsar {
namespace UI {

void PatchGlobeSearchBMG(Pages::GlobeSearch* globeSearch) {
    globeSearch->countdown.Update();
    if(Pulsar::System::sInstance->deniesCount >= 3) globeSearch->messageWindow.LayoutUIControl::SetMsgId(UI::BMG_TOO_MANY_DENIES);
}
kmCall(0x8060926c, PatchGlobeSearchBMG);

}//namespace UI
}//namespace Pulsar