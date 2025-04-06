#include <MarioKartWii/UI/Page/Other/GlobeSearch.hpp>
#include <MarioKartWii/RKSYS/RKSYSMgr.hpp>
#include <Settings/UI/ExpWFCMainPage.hpp>
#include <UI/UI.hpp>
// #include <UI/PlayerCount.hpp>
#include <PulsarSystem.hpp>

namespace Pulsar {
namespace UI {
//EXPANDED WFC, keeping WW button and just hiding it in case it is ever needed...

kmWrite32(0x8064b984, 0x60000000); //nop the InitControl call in the init func
kmWrite24(0x80899a36, 'PUL'); //8064ba38
kmWrite24(0x80899a5B, 'PUL'); //8064ba90

void ExpWFCMain::OnInit() {
    this->InitControlGroup(6); //5 controls usually + settings button
    WFCMainMenu::OnInit();
    this->AddControl(5, settingsButton, 0);

    this->settingsButton.Load(UI::buttonFolder, "Settings1P", "Settings", 1, 0, false);
    this->settingsButton.buttonId = 5;
    this->settingsButton.SetOnClickHandler(this->onSettingsClick, 0);
    this->settingsButton.SetOnSelectHandler(this->onButtonSelectHandler);

    this->topSettingsPage = SettingsPanel::id;
}

void ExpWFCMain::OnSettingsButtonClick(PushButton& pushButton, u32 r5) {
    ExpSection::GetSection()->GetPulPage<SettingsPanel>()->prevPageId = PAGE_WFC_MAIN;
    this->nextPageId = static_cast<PageId>(this->topSettingsPage);
    this->EndStateAnimated(0, pushButton.GetAnimationFrameSize());
}

void ExpWFCMain::ExtOnButtonSelect(PushButton& button, u32 hudSlotId) {
    if(button.buttonId == 5) {
        u32 bmgId = BMG_SETTINGS_BOTTOM + 1;
        if(this->topSettingsPage == PAGE_VS_TEAMS_VIEW) bmgId += 1;
        else if(this->topSettingsPage == PAGE_BATTLE_MODE_SELECT) bmgId += 2;
        this->bottomText.SetMessage(bmgId, 0);
    }
    else this->OnButtonSelect(button, hudSlotId);
}

//ExpWFCModeSel
kmWrite32(0x8064c284, 0x38800001); //distance func

void ExpWFCModeSel::OnInit() {
    WFCModeSelect::OnInit();
    // this->manipulatorManager.SetGlobalHandler(START_PRESS, this->onStartPress, false, false);
}

void ExpWFCModeSel::InitButton(ExpWFCModeSel& self) {
    self.InitControlGroup(7);

    self.region = 0x521;  // Store region in the page class instead
    self.AddControl(5, self.ottButton, 0);
    self.ottButton.Load(UI::buttonFolder, "PULOTTButton", "PULOTTButton", 1, 0, 0);
    self.ottButton.buttonId = ottButtonId;
    self.ottButton.SetOnClickHandler(self.onModeButtonClickHandler, 0);
    self.ottButton.SetOnSelectHandler(self.onButtonSelectHandler);

    self.AddControl(6, self.twoHundredButton, 0);
    self.twoHundredButton.Load(UI::buttonFolder, "PUL200Button", "PUL200Button", 1, 0, 0);
    self.twoHundredButton.buttonId = twoHundredButtonId;
    self.twoHundredButton.SetMessage(BMG_200_BUTTON);
    self.twoHundredButton.SetOnClickHandler(self.onModeButtonClickHandler, 0);
    self.twoHundredButton.SetOnSelectHandler(self.onButtonSelectHandler);
    
    Text::Info info;
    RKSYS::Mgr* rksysMgr = RKSYS::Mgr::sInstance;
    u32 vr = 0;
    if(rksysMgr->curLicenseId >= 0) {
        RKSYS::LicenseMgr& license = rksysMgr->licenses[rksysMgr->curLicenseId];
        vr = license.vr.points;
    }
    info.intToPass[0] = vr;
    self.ottButton.SetTextBoxMessage("go", BMG_RATING, &info);
    self.twoHundredButton.SetTextBoxMessage("go", BMG_RATING, &info);
}
kmCall(0x8064c294, ExpWFCModeSel::InitButton);

void ExpWFCModeSel::ClearModeContexts() {
    const u32 modeContexts[] = {
        PULSAR_MODE_OTT,
        PULSAR_200_WW,
    };
    
    const u32 numContexts = sizeof(modeContexts) / sizeof(modeContexts[0]);
    for(u32 i = 0; i < numContexts; ++i) {
        u32 context = modeContexts[i];
        System::sInstance->context &= ~(1 << context);
    }
}

void ExpWFCModeSel::OnModeButtonClick(PushButton& modeButton, u32 hudSlotId) {
    const u32 id = modeButton.buttonId;
    ClearModeContexts();
    
    if (id == ottButtonId) {
        System::sInstance->netMgr.region = 0x522;
        System::sInstance->context |= (1 << PULSAR_MODE_OTT);
    }
    else if (id == twoHundredButtonId) {
        System::sInstance->netMgr.region = 0x521;
        System::sInstance->context |= (1 << PULSAR_200_WW);
    }
    else {
        System::sInstance->netMgr.region = 0x520;
    }

    this->lastClickedButton = id;
    WFCModeSelect::OnModeButtonClick(modeButton, hudSlotId);
}

void ExpWFCModeSel::OnActivatePatch() { 
    register ExpWFCModeSel* page;
    asm(mr page, r29;);
    register Pages::GlobeSearch* search;
    asm(mr search, r30;);
    const bool isHidden = search->searchType == 1 ? false : true; //make the button visible if continental was clicked

    // Reset game mode if worldwide is selected
    if (isHidden) {
        ClearModeContexts();
        System::sInstance->netMgr.region = 0x520;
        page->lastClickedButton = 0; // Reset to VS button
    }

    page->ottButton.isHidden = isHidden;
    page->ottButton.manipulator.inaccessible = isHidden;
    page->twoHundredButton.isHidden = isHidden;
    page->twoHundredButton.manipulator.inaccessible = isHidden;

    page->battleButton.isHidden = true;
    page->battleButton.manipulator.inaccessible = true;

    page->nextPage = PAGE_NONE;
    PushButton* button = &page->vsButton;
    u32 bmgId = UI::BMG_RACE_WITH11P;

    // Determine which button should be selected based on current context
    if(System::sInstance->IsContext(PULSAR_MODE_OTT)) {
        page->lastClickedButton = ottButtonId;
        button = &page->ottButton;
        bmgId = UI::BMG_OTT_WW_BOTTOM;
    }
    else if(System::sInstance->IsContext(PULSAR_200_WW)) {
        page->lastClickedButton = twoHundredButtonId;
        button = &page->twoHundredButton;
        bmgId = UI::BMG_200_WW_BOTTOM;
    }
    else if(page->lastClickedButton == 2) {
        button = &page->battleButton;
        bmgId = UI::BMG_BATTLE_WITH6P;
    }

    page->bottomText.SetMessage(bmgId);
    button->SelectInitial(0);
}
kmCall(0x8064c5f0, ExpWFCModeSel::OnActivatePatch);

void ExpWFCModeSel::OnModeButtonSelect(PushButton& modeButton, u32 hudSlotId) {
    if(modeButton.buttonId == ottButtonId) {
        this->bottomText.SetMessage(BMG_OTT_WW_BOTTOM);
    }   
    else if(modeButton.buttonId == twoHundredButtonId) {
        this->bottomText.SetMessage(BMG_200_WW_BOTTOM);
    }
    else WFCModeSelect::OnModeButtonSelect(modeButton, hudSlotId);
}

} // namespace UI
} // namespace Pulsar