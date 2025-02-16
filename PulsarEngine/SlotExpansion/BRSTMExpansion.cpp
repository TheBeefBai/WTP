#include <kamek.hpp>
#include <MarioKartWii/Audio/AudioManager.hpp>
#include <MarioKartWii/UI/Section/SectionMgr.hpp>
#include <Sound/MiscSound.hpp>
#include <SlotExpansion/CupsConfig.hpp>
#include <SlotExpansion/UI/ExpansionUIMisc.hpp>


namespace Pulsar {
namespace Sound {
//Custom implementation of music slot expansion; this would break with regs
//kmWrite32(0x8009e0dc, 0x7F87E378); //mr r7, r28 to get string length

static char pulPath[0x100];
s32 CheckBRSTM(const nw4r::snd::DVDSoundArchive* archive, PulsarId id, u8 variantIdx, bool isFinalLap) {

    const char* root = archive->extFileRoot;
    const char* lapSpecifier = isFinalLap ? "_f" : "_n";
    s32 ret = -1;
    char trackName[0x100];
    UI::GetTrackBMG(trackName, id);
    snprintf(pulPath, 0x100, "%sstrm/%s%s.brstm", root, trackName, lapSpecifier);
    ret = DVD::ConvertPathToEntryNum(pulPath);
    if(ret < 0) {
        u32 variantIdx = CupsConfig::sInstance->GetCurVariantIdx();
        if(variantIdx == 0) snprintf(pulPath, 0x50, "%sstrm/%d%s.brstm", root, CupsConfig::ConvertTrack_PulsarIdToRealId(id), lapSpecifier);
        else snprintf(pulPath, 0x50, "%sstrm/%d_%d%s.brstm", root, CupsConfig::ConvertTrack_PulsarIdToRealId(id), variantIdx, lapSpecifier);
        ret = DVD::ConvertPathToEntryNum(pulPath);
    }
    return ret;
}

nw4r::ut::FileStream* MusicSlotsExpand(nw4r::snd::DVDSoundArchive* archive, void* buffer, int size,
    const char* extFilePath, u32 r7, u32 length) {

    const char firstChar = extFilePath[0xC];
    const CupsConfig* cupsConfig = CupsConfig::sInstance;
    const PulsarId track = cupsConfig->GetWinning();

    if((firstChar == 'n' || firstChar == 'S' || firstChar == 'r')) {
        const SectionId section = SectionMgr::sInstance->curSection->sectionId;
        register SoundIDs toPlayId;
        asm(mr toPlayId, r20;);
        const char* customBGPath = nullptr;
        if(toPlayId == SOUND_ID_KC) { //files are guaranteed to exist because it's been checked before
            if(section >= SECTION_MAIN_MENU_FROM_BOOT && section <= SECTION_MAIN_MENU_FROM_LICENSE) customBGPath = titleMusicFile;
            else if(section >= SECTION_SINGLE_P_FROM_MENU && section <= SECTION_SINGLE_P_LIST_RACE_GHOST || section == SECTION_LOCAL_MULTIPLAYER) customBGPath = offlineMusicFile;
            else if(section >= SECTION_P1_WIFI && section <= SECTION_P2_WIFI_FROOM_COIN_VOTING) customBGPath = wifiMusicFile;
        }
        if(customBGPath != nullptr) extFilePath = customBGPath;
        else if(!CupsConfig::IsReg(track)) {
            bool isFinalLap = false;
            register u32 strLength;
            asm(mr strLength, r28;);
            const char finalChar = extFilePath[strLength];
            if(finalChar == 'f' || finalChar == 'F') isFinalLap = true;

            bool found = false;
            const u8 variantIdx = cupsConfig->GetCurVariantIdx();
            if(CheckBRSTM(archive, track, variantIdx, isFinalLap) >= 0) found = true;
            else if(isFinalLap) {
                if(CheckBRSTM(archive, track, variantIdx, false) >= 0) found = true;
                if(found) Audio::Manager::sInstance->soundArchivePlayer->soundPlayerArray->soundList.GetFront().ambientParam.pitch = 1.1f;
            }
            if(found) extFilePath = pulPath;
        }
    }
    return archive->OpenExtStream(buffer, size, extFilePath, 0, length);
}
kmCall(0x8009e0e4, MusicSlotsExpand);

}//namespace Sound
}//namespace Pulsar