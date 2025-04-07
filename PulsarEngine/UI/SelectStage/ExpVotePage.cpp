#include <UI/SelectStage/ExpVotePage.hpp>
#include <MarioKartWii/UI/Section/SectionMgr.hpp>
#include <MarioKartWii/RKNet/RKNetController.hpp>
#include <MarioKartWii/UI/Page/Other/SELECTStageMgr.hpp>
#include <core/rvl/OS/OS.hpp>
#include <PulsarSystem.hpp>

namespace Pulsar {
namespace UI {

void ExpVotePage::BeforeControlUpdate() {
    if (this->currentState == STATE_ACTIVE && this->connectionState != 3) {
        if (System::sInstance->IsContext(PULSAR_MODE_KO)) {
            u32 aidSave = RKNet::Controller::sInstance->subs[RKNet::Controller::sInstance->currentSub].availableAids;
            RKNet::Controller::sInstance->subs[RKNet::Controller::sInstance->currentSub].availableAids = aidSave | 1;
            Pages::Vote::BeforeControlUpdate();
            RKNet::Controller::sInstance->subs[RKNet::Controller::sInstance->currentSub].availableAids = aidSave;
            return;
        }
    }

    Pages::Vote::BeforeControlUpdate();
}

} // namespace UI
} // namespace Pulsar
