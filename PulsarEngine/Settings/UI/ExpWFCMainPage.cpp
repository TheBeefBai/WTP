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
    self.InitControlGroup(9);

    self.region = 0x521;  // Store region in the page class instead
    self.AddControl(5, self.ottButton, 0);
    self.ottButton.Load(UI::buttonFolder, "XifiMiscMenuModeSelect", "OTTButton", 1, 0, 0);
    self.ottButton.buttonId = ottButtonId;
    self.ottButton.SetMessage(BMG_OTT_BUTTON);
    self.ottButton.SetOnClickHandler(self.onModeButtonClickHandler, 0);
    self.ottButton.SetOnSelectHandler(self.onButtonSelectHandler);

    self.AddControl(6, self.twoHundredButton, 0);
    self.twoHundredButton.Load(UI::buttonFolder, "XifiMiscMenuModeSelect", "200Button", 1, 0, 0);
    self.twoHundredButton.buttonId = twoHundredButtonId;
    self.twoHundredButton.SetMessage(BMG_200_BUTTON);
    self.twoHundredButton.SetOnClickHandler(self.onModeButtonClickHandler, 0);
    self.twoHundredButton.SetOnSelectHandler(self.onButtonSelectHandler);

    self.AddControl(7, self.itemRainButton, 0);
    self.itemRainButton.Load(UI::buttonFolder, "XifiMiscMenuModeSelect", "ItemRainButton", 1, 0, 0);
    self.itemRainButton.buttonId = itemRainButtonId;
    self.itemRainButton.SetMessage(BMG_ITEMRAIN_BUTTON);
    self.itemRainButton.SetOnClickHandler(self.onModeButtonClickHandler, 0);
    self.itemRainButton.SetOnSelectHandler(self.onButtonSelectHandler);

    self.AddControl(8, self.shockButton, 0);
    self.shockButton.Load(UI::buttonFolder, "XifiMiscMenuModeSelect", "ShockButton", 1, 0, 0);
    self.shockButton.buttonId = shockButtonId;
    self.shockButton.SetMessage(BMG_SHOCK_BUTTON);
    self.shockButton.SetOnClickHandler(self.onModeButtonClickHandler, 0);
    self.shockButton.SetOnSelectHandler(self.onButtonSelectHandler);
    
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
    self.itemRainButton.SetTextBoxMessage("go", BMG_RATING, &info);
    self.shockButton.SetTextBoxMessage("go", BMG_RATING, &info);
}
kmCall(0x8064c294, ExpWFCModeSel::InitButton);

void ExpWFCModeSel::ClearModeContexts() {
    const u32 modeContexts[] = {
        PULSAR_MODE_OTT,
        PULSAR_200_WW,
        PULSAR_GAMEMODEITEMRAIN,
        PULSAR_GAMEMODESHOCK,
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
    else if (id == itemRainButtonId) {
        System::sInstance->netMgr.region = 0x523;
        System::sInstance->context |= (1 << PULSAR_GAMEMODEITEMRAIN);
    }
    else if (id == shockButtonId) {
        System::sInstance->netMgr.region = 0x524;
        System::sInstance->context |= (1 << PULSAR_GAMEMODESHOCK);
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

    page->ottButton.SetPaneVisibility("capsul_null", false);
    page->twoHundredButton.SetPaneVisibility("capsul_null", false);
    page->itemRainButton.SetPaneVisibility("capsul_null", false);
    page->shockButton.SetPaneVisibility("capsul_null", false);

    page->ottButton.isHidden = isHidden;
    page->ottButton.manipulator.inaccessible = isHidden;
    page->twoHundredButton.isHidden = isHidden;
    page->twoHundredButton.manipulator.inaccessible = isHidden;
    page->itemRainButton.isHidden = isHidden;
    page->itemRainButton.manipulator.inaccessible = isHidden;
    page->shockButton.isHidden = isHidden;
    page->shockButton.manipulator.inaccessible = isHidden;

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
    else if(System::sInstance->IsContext(PULSAR_GAMEMODEITEMRAIN)) {
        page->lastClickedButton = itemRainButtonId;
        button = &page->itemRainButton;
        bmgId = UI::BMG_ITEMRAIN_WW_BOTTOM;
    }
    else if(System::sInstance->IsContext(PULSAR_GAMEMODESHOCK)) {
        page->lastClickedButton = shockButtonId;
        button = &page->shockButton;
        bmgId = UI::BMG_SHOCK_WW_BOTTOM;
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
    else if(modeButton.buttonId == itemRainButtonId) {
        this->bottomText.SetMessage(BMG_ITEMRAIN_WW_BOTTOM);
    }
    else if(modeButton.buttonId == shockButtonId) {
        this->bottomText.SetMessage(BMG_SHOCK_WW_BOTTOM);
    }
    else WFCModeSelect::OnModeButtonSelect(modeButton, hudSlotId);
}

} // namespace UI
} // namespace Pulsar