#ifndef _PUL_WFC_
#define _PUL_WFC_
#include <kamek.hpp>
#include <MarioKartWii/UI/Page/Other/WFCMenu.hpp>
#include <Settings/UI/SettingsPanel.hpp>

//Extends WFCMainMenu to add a settings button
namespace Pulsar {
namespace UI {

static bool s_displayPlayerCount = true;

class ExpWFCMain : public Pages::WFCMainMenu {
public:
    ExpWFCMain() {
        this->onSettingsClick.subject = this;
        this->onSettingsClick.ptmf = &ExpWFCMain::OnSettingsButtonClick;
        this->onButtonSelectHandler.ptmf = &ExpWFCMain::ExtOnButtonSelect;

        // this->onStartPress.subject = this;
        // this->onStartPress.ptmf = &ExpWFCMain::ExtOnStartPress;
    }
    void OnInit() override;
    // void BeforeControlUpdate() override;
private:
    void OnSettingsButtonClick(PushButton& PushButton, u32 r5);
    void ExtOnButtonSelect(PushButton& pushButton, u32 hudSlotId);
    // void ExtOnStartPress(u32 hudSlotId) {
    //     s_displayPlayerCount = !s_displayPlayerCount;
    // }

    PtmfHolder_2A<ExpWFCMain, void, PushButton&, u32> onSettingsClick;
    // PtmfHolder_1A<ExpWFCMain, void, u32> onStartPress;
    PushButton settingsButton;
public:
    PulPageId topSettingsPage;
};

class ExpWFCModeSel : public Pages::WFCModeSelect {
public:
    ExpWFCModeSel() : lastClickedButton(0), region(0xA) {
        this->onButtonSelectHandler.ptmf = &ExpWFCModeSel::OnModeButtonSelect;
        this->onModeButtonClickHandler.ptmf = &ExpWFCModeSel::OnModeButtonClick;

        // this->onStartPress.subject = this;
        // this->onStartPress.ptmf = &ExpWFCModeSel::ExtOnStartPress;
    }
    void OnInit() override;
    // void BeforeControlUpdate() override;
    static void InitButton(ExpWFCModeSel& self);
    static void OnActivatePatch();
    static void ClearModeContexts();
public:
    void OnModeButtonSelect(PushButton& modeButton, u32 hudSlotId); //8064c718
    void OnModeButtonClick(PushButton& PushButton, u32 r5);
    // void ExtOnStartPress(u32 hudSlotId) {
    //     s_displayPlayerCount = !s_displayPlayerCount;
    // }

    // PtmfHolder_1A<ExpWFCModeSel, void, u32> onStartPress;

    PushButton ottButton;
    PushButton twoHundredButton;
    u32 lastClickedButton;
    u32 region;
    static const u32 ottButtonId = 4;
    static const u32 twoHundredButtonId = 5;
};
}//namespace UI
}//namespace Pulsar

#endif