#include <MarioKartWii/Race/Raceinfo/Raceinfo.hpp>
#include <MarioKartWii/UI/Page/Page.hpp>
#include <MarioKartWii/Input/InputManager.hpp>
#include <GameModes/KO/KOMgr.hpp>
#include <Network/PacketExpansion.hpp>
#include <Gamemodes/KO/KORaceEndPage.hpp>
#include <Settings/Settings.hpp>

//WTP Dev Note: Fixes by Retro Rewind Team

namespace Pulsar {
namespace KO {

Mgr::Mgr() : winnerPlayerId(0xFF), isSpectating(false), hasSwapped(false) /*, stillInCount(playerCount)*/ {
    const RKNet::Controller* controller = RKNet::Controller::sInstance;
    const RKNet::ControllerSub& sub = controller->subs[controller->currentSub];
    this->baseLocPlayerCount = sub.localPlayerCount;
    for(int aid = 0; aid < 12; ++aid) {
        this->status[aid][0] = NORMAL;
        this->status[aid][1] = NORMAL;
    }
    this->ResetRace();
}
Mgr::~Mgr() {
    RKNet::Controller* controller = RKNet::Controller::sInstance;
    controller->subs[0].localPlayerCount = this->baseLocPlayerCount;
    controller->subs[1].localPlayerCount = this->baseLocPlayerCount;
    if(this->GetIsSwapped()) this->SwapControllersAndUI();
}

void Mgr::AddRaceStats() { //SHOULD ONLY BE CALLED AFTER PROCESSKOS
    //CALC THE STATS
    const RacedataScenario& scenario = Racedata::sInstance->racesScenario;
    for(int hudSlot = 0; hudSlot < scenario.localPlayerCount; ++hudSlot) {
        Stats& stats = this->stats[hudSlot];
        if(stats.boolCountArray >= arbitraryAlmostDied) ++stats.final.almostKOdCounter;
        const u8 pos = Raceinfo::sInstance->players[scenario.settings.hudPlayerIds[hudSlot]]->position;
        stats.percentageSum += static_cast<float>(pos) / static_cast<float>(System::sInstance->nonTTGhostPlayersCount); //this allows higher precision across multiple races
        stats.final.finalPercentageSum = static_cast<u8>(stats.percentageSum * 100);
    }

    this->ResetRace();
}

void Mgr::CalcWouldBeKnockedOut() {
    // Initialize players
    Pages::GPVSLeaderboardUpdate::Player players[12];
    memset( & players, 0, sizeof(players));
    const RacedataScenario & scenario = Racedata::sInstance -> menusScenario;
    const RKNet::Controller * controller = RKNet::Controller::sInstance;
    const RKNet::ControllerSub & sub = controller -> subs[controller -> currentSub];
    const u8 playerCount = System::sInstance -> nonTTGhostPlayersCount;
    const Raceinfo * raceInfo = Raceinfo::sInstance;

    const u8 * pointsArray = & Racedata::pointsRoom[playerCount - 1][0];

    u32 disconnectedKOs = 0;
    for (int curPlayerId = 0; curPlayerId < playerCount; ++curPlayerId) { // Initialize players struct
        this -> wouldBeOut[curPlayerId] = false;
        const u8 aid = controller -> aidsBelongingToPlayerIds[curPlayerId];
        if ((1 << aid & sub.availableAids) == 0) ++disconnectedKOs;
        Pages::GPVSLeaderboardUpdate::Player & cur = players[curPlayerId];
        const u8 wouldBePoints = pointsArray[raceInfo -> players[curPlayerId] -> position - 1];
        cur.lastRaceScore = wouldBePoints;
        cur.totalScore = scenario.players[curPlayerId].previousScore + wouldBePoints;
        cur.playerId = curPlayerId;
    }

    if (playerCount == 2 || (playerCount - disconnectedKOs) == 1) { // Use position-based KOs
        for (int i = 0; i < playerCount; ++i) {
            if (raceInfo -> players[i] -> position == 1) {
                this -> wouldBeOut[i] = false; // Winner is not knocked out
            } else {
                this -> wouldBeOut[i] = true;
            }
        }
    } else {
        // Sort players
        qsort(players, playerCount, sizeof(Pages::GPVSLeaderboardUpdate::Player), reinterpret_cast < int( * )(const void * ,
            const void * ) > ( & Pages::GPVSLeaderboardTotal::ComparePlayers));

        // Calculate real KO count
        const u32 theoreKOs = this -> koPerRace - ((playerCount - this -> koPerRace == 1) && this -> alwaysFinal);
        const s32 realKOCount = theoreKOs - disconnectedKOs;

        if (realKOCount > 0 && (SectionMgr::sInstance -> sectionParams -> onlineParams.currentRaceNumber + 1) % this -> racesPerKO == 0) {
            int koAssigned = 0;
            for (int idx = playerCount - 1 - disconnectedKOs; idx >= 0 && koAssigned < realKOCount; --idx) {
                u32 position = idx;
                u8 playerId;
                if (racesPerKO == 1) {
                    playerId = raceInfo -> playerIdInEachPosition[position];
                } else {
                    playerId = players[position].playerId;
                }

                // Skip the winner
                if (playerId == winnerPlayerId || raceInfo -> players[playerId] -> position == 1) {
                    continue;
                }

                // Check if the context is PULSAR_KOFINAL and KOSETTING_FINAL_ALWAYS is true
                if (System::sInstance -> IsContext(Pulsar::PULSAR_KOFINAL) == KOSETTING_FINAL_ALWAYS && playerCount > 2) {
                    if (players[0].playerId == playerId || players[1].playerId == playerId) {
                        continue; // Do not KO the top 2 racers
                    }
                }

                this -> wouldBeOut[playerId] = true;
                ++koAssigned;
            }
        }
    }
}

void Mgr::ProcessKOs(Pages::GPVSLeaderboardUpdate::Player * playerArr, size_t nitems, size_t size, int( * compar)(const void * ,
    const void * )) {
    qsort(playerArr, nitems, size, compar); // default sort

    const System * system = System::sInstance;
    if (system -> IsContext(PULSAR_MODE_KO)) {
        Mgr * self = system -> koMgr;

        RacedataScenario & scenario = Racedata::sInstance -> menusScenario;
        const RKNet::Controller * controller = RKNet::Controller::sInstance;
        const RKNet::ControllerSub & sub = controller -> subs[controller -> currentSub];
        const u8 playerCount = system -> nonTTGhostPlayersCount;

        u8 disconnectedKOs = 0;
        for (int playerId = 0; playerId < playerCount; ++playerId) { // eliminate disconnected players
            const u8 aid = controller -> aidsBelongingToPlayerIds[playerId];
            if ((1 << aid & sub.availableAids) == 0) {
                self -> SetKOd(playerId);
                ++disconnectedKOs;
            }
        }

        u8 theoreKOs = self -> koPerRace - ((playerCount - self -> koPerRace == 1) && self -> alwaysFinal);
        s8 realKOCount = theoreKOs - disconnectedKOs;

        const Raceinfo * raceinfo = Raceinfo::sInstance;
        bool hasTies = false;

        if (self -> koPerRace >= 2 && System::sInstance -> IsContext(Pulsar::PULSAR_KOFINAL) == KOSETTING_FINAL_ALWAYS && playerCount > 2) {
            if (playerCount == 3) {
                realKOCount = 1;
            } else if (playerCount == 4 && self -> koPerRace >= 2) {
                realKOCount = 2;
            }
        } else {
            if (playerCount == 3 && self -> koPerRace >= 3) {
                realKOCount = 2;
            } else if (playerCount == 4 && self -> koPerRace >= 4) {
                realKOCount = 3;
            }
        }

        if (playerCount == 2 || (playerCount - disconnectedKOs) == 1) {
            self -> winnerPlayerId = raceinfo -> playerIdInEachPosition[0];
            self -> SetKOd(raceinfo -> playerIdInEachPosition[1]);
        } else if (realKOCount > 0 && (SectionMgr::sInstance -> sectionParams -> onlineParams.currentRaceNumber + 1) % self -> racesPerKO == 0) {

            if (self -> racesPerKO > 1) {
                u32 highestKOPosition = playerCount - realKOCount;
                u32 tieScore = playerArr[highestKOPosition].totalScore;
                int tiedPlayersCount = 0;
                int playersInKOPosition = 0;
                int playersNotInKOPosition = 0;

                // Count tied players and determine if they are in KO positions
                for (int position = 0; position < playerCount; ++position) {
                    if (playerArr[position].totalScore == tieScore) {
                        ++tiedPlayersCount;
                        if (position >= playerCount - realKOCount) {
                            ++playersInKOPosition;
                        } else {
                            ++playersNotInKOPosition;
                        }
                    }
                }

                if (playersInKOPosition > 0 && playersNotInKOPosition > 0) {
                    // Tied players with some in KO position and some not, set tie breaker race
                    for (int position = 0; position < playerCount; ++position) {
                        if (playerArr[position].totalScore == tieScore) {
                            self -> SetTie(playerArr[position].playerId, playerArr[highestKOPosition].playerId);
                            hasTies = true;
                        }
                    }
                    if (hasTies) {
                        realKOCount = 0;
                        --SectionMgr::sInstance -> sectionParams -> onlineParams.currentRaceNumber;
                    }
                } else if (tiedPlayersCount > realKOCount) {
                    // KO players in KO positions who did not tie
                    for (int position = playerCount - realKOCount; position < playerCount; ++position) {
                        if (playerArr[position].totalScore != tieScore) {
                            self -> SetKOd(playerArr[position].playerId);
                            --realKOCount;
                        }
                    }

                    // Set tie breaker race for tied players
                    for (int position = 0; position < playerCount; ++position) {
                        if (playerArr[position].totalScore == tieScore) {
                            self -> SetTie(playerArr[position].playerId, playerArr[highestKOPosition].playerId);
                            hasTies = true;
                        }
                    }
                    if (hasTies) {
                        realKOCount = 0;
                        --SectionMgr::sInstance -> sectionParams -> onlineParams.currentRaceNumber;
                    }
                } else if (tiedPlayersCount == realKOCount) {
                    // All tied players are in elimination positions, KO all
                    for (int position = 0; position < playerCount; ++position) {
                        if (playerArr[position].totalScore == tieScore) {
                            self -> SetKOd(playerArr[position].playerId);
                        }
                    }
                    realKOCount = tiedPlayersCount;
                    --SectionMgr::sInstance -> sectionParams -> onlineParams.currentRaceNumber;
                } else {
                    // Proceed to KO players in elimination positions
                    for (int idx = playerCount - realKOCount; idx < playerCount; ++idx) {
                        self -> SetKOd(playerArr[idx].playerId);
                    }
                }
            }

            // Check for tie breaker race condition
            if (playerArr[0].totalScore == playerArr[1].totalScore && (self -> IsKOdPlayerId(playerArr[0].playerId) || self -> IsKOdPlayerId(playerArr[1].playerId))) {
                self -> SetTie(playerArr[0].playerId, playerArr[1].playerId);
                hasTies = true;
            }

            if (System::sInstance -> IsContext(Pulsar::PULSAR_KOFINAL) == KOSETTING_FINAL_ALWAYS && playerCount > 2) {
                if (playerArr[1].totalScore == playerArr[2].totalScore && (self -> IsKOdPlayerId(playerArr[1].playerId) || self -> IsKOdPlayerId(playerArr[2].playerId))) {
                    self -> SetTie(playerArr[1].playerId, playerArr[2].playerId);
                    hasTies = true;
                }
            }

            // KO players in elimination positions if no ties
            if (realKOCount > 0 && hasTies == false) {
                int koCount = 0;
                for (int idx = playerCount - 1 - disconnectedKOs; idx >= 0 && koCount < realKOCount; --idx) {
                    u8 playerId;
                    if (self -> racesPerKO == 1) {
                        playerId = raceinfo -> playerIdInEachPosition[idx];
                    } else {
                        playerId = playerArr[idx].playerId;
                    }

                    // Skip the winner and the player in first position
                    if (playerId == self -> winnerPlayerId || raceinfo -> players[playerId] -> position == 1) {
                        continue;
                    }

                    // Check if the context is PULSAR_KOFINAL and KOSETTING_FINAL_ALWAYS is true
                    if (System::sInstance -> IsContext(Pulsar::PULSAR_KOFINAL) == KOSETTING_FINAL_ALWAYS && playerCount > 2) {
                        if (playerArr[0].playerId == playerId || playerArr[1].playerId == playerId) {
                            continue; // Do not KO the top 2 racers
                        }
                    }

                    self -> SetKOd(playerId);
                    ++koCount;
                }
            }

            // Reset the scores if no ties AND more than 1 race per KO AND there are KOs
            if (realKOCount > 0 && hasTies == false && self -> racesPerKO > 1) {
                for (int idx = 0; idx < 12; ++idx) {
                    scenario.players[idx].score = 0;
                    scenario.players[idx].previousScore = 0;
                }
            }
        }

        // Check if only one player is not knocked out and declare them the winner
        int notKOdCount = 0;
        u8 potentialWinner = 0xFF;
        for (int playerId = 0; playerId < playerCount; ++playerId) {
            if (!self -> IsKOdPlayerId(playerId)) {
                ++notKOdCount;
                potentialWinner = playerId;
            }
        }
        if (notKOdCount == 1) {
            self -> winnerPlayerId = potentialWinner;
        }

        self -> AddRaceStats(); // Add stats after processing
    }
}
kmCall(0x8085cb94, Mgr::ProcessKOs);

void Mgr::Update() {
    const System* system = System::sInstance;
    if(system->IsContext(PULSAR_MODE_KO)) {
        Mgr* self = System::sInstance->koMgr;
        self->CalcWouldBeKnockedOut();
        const RacedataScenario& scenario = Racedata::sInstance->racesScenario;
        for(int hudSlot = 0; hudSlot < scenario.localPlayerCount; ++hudSlot) {
            const bool wouldBeOut = self->GetWouldBeKnockedOut(scenario.settings.hudPlayerIds[hudSlot]);
            const u32 idx = Raceinfo::sInstance->raceFrames % 300;

            Stats& stats = self->stats[hudSlot];
            if(wouldBeOut) ++stats.final.timeInDanger;
            if(stats.isInDangerFrames[idx] == false && wouldBeOut == true) ++stats.boolCountArray;
            else if(stats.isInDangerFrames[idx] == true && wouldBeOut == false) --stats.boolCountArray;
            stats.isInDangerFrames[idx] = wouldBeOut;
        }

        const u8 winnerPlayerId = self->winnerPlayerId;
        if(winnerPlayerId != 0xFF) { //if the if is taken, ProcessKOs and therefore AddRaceStats are guaranteed to have been called
            const RKNet::Controller* controller = RKNet::Controller::sInstance;
            const RKNet::ControllerSub& sub = controller->subs[controller->currentSub];
            if(controller->aidsBelongingToPlayerIds[winnerPlayerId] == sub.localAid) { //only send the data if needed 
                for(int aid = 0; aid < 12; ++aid) {
                    if((1 << aid & sub.availableAids) == 0 || aid == sub.localAid) continue;

                    Stats& stats = self->stats[0];
                    RKNet::PacketHolder<Network::PulRH1>* holder = controller->GetSendPacketHolder<Network::PulRH1>(aid);

                    /*
                    this shouldn't be needed because the "ExportRH1ToPulRH1" always does it
                    const u32 curSize = holder->packetSize;/
                    u32 addedSize = 0;
                    if(curSize == sizeof(RKNet::RACEHEADER1Packet)) addedSize = sizeof(Network::PulRH1) - sizeof(RKNet::RACEHEADER1Packet);
                    else if(holder->packetSize == 0) addedSize = sizeof(Network::PulRH1);
                    holder->packetSize += addedSize;
                    */
                    Network::PulRH1* dest = holder->packet;
                    dest->timeInDanger = stats.final.timeInDanger;
                    dest->almostKOdCounter = stats.final.almostKOdCounter;
                    dest->finalPercentageSum = stats.final.finalPercentageSum;
                }
            }
        }
    }
}
RaceFrameHook koUpdate(Mgr::Update);

/*
void Mgr::UpdateStillInCount() {
    u32 stillIn = 0;
    const RKNet::Controller* controller = RKNet::Controller::sInstance;
    const u32 availableAids =  controller->subs[controller->currentSub].availableAids;
    for(int aid = 0; aid < 12; ++aid) {
        u8 aid = controller->aidsBelongingToPlayerIds[aid];
        if((1 << aid & availableAids) == 0) { //the playerId is disconnected
            this->SetMgrd(aid);
        }
        if(this->IsMgrd(aid)) continue;
        ++stillIn;
    }
    this->stillInCount = stillIn;
}
*/

void Mgr::PatchAids(RKNet::ControllerSub& sub) const {
    u32 availableAids = sub.availableAids;
    const u8 localAid = sub.localAid;
    for(u8 aid = 0; aid < 12; ++aid) {

        bool isConsoleOut = false;
        const bool isMainOut = this->IsKOdAid(aid, 0);
        u8 aidPlayerCount = aid == localAid ? sub.localPlayerCount : sub.connectionUserDatas[aid].playersAtConsole;


        if(aidPlayerCount <= 1) isConsoleOut = isMainOut;
        else if(aidPlayerCount == 2) {
            const bool isGuestOut = this->IsKOdAid(aid, 1);
            if(isMainOut && isGuestOut) isConsoleOut = true;
            else if(isMainOut != isGuestOut) {
                aidPlayerCount = 1;
            }

        }
        if(isConsoleOut) {
            availableAids = availableAids & ~(1 << aid);
            aidPlayerCount = 0;
        }

        if(aid == localAid) sub.localPlayerCount = aidPlayerCount;
        else sub.connectionUserDatas[aid].playersAtConsole = aidPlayerCount;
    }
    sub.availableAids = availableAids;
}

u32 Mgr::GetAidAndSlotFromPlayerId(u8 playerId) const {
    const RKNet::Controller* controller = RKNet::Controller::sInstance;
    const RKNet::ControllerSub& sub = controller->subs[controller->currentSub];
    const u8 aid = controller->aidsBelongingToPlayerIds[playerId];
    const u8 localAid = sub.localAid;
    u8 slot = 0;

    if((aid == localAid && sub.localPlayerCount == 2) || (aid != localAid && sub.connectionUserDatas[aid].playersAtConsole == 2)) {
        if(playerId > 0 && controller->aidsBelongingToPlayerIds[playerId - 1] == aid) slot = 1;
        else if(playerId < 11 && this->status[aid][0] == KOD && controller->aidsBelongingToPlayerIds[playerId + 1] != aid) slot = 1;
    }
    return (slot << 16) | aid; //10001
}

SectionId Mgr::GetSectionAfterKO(SectionId defaultId) const {
    const RKNet::Controller* controller = RKNet::Controller::sInstance;
    const RKNet::ControllerSub& sub = controller->subs[controller->currentSub];
    if(this->baseLocPlayerCount == 2) {
        if(this->IsKOdAid(sub.localAid, 0) != this->IsKOdAid(sub.localAid, 1)) {
            if(defaultId == SECTION_P2_WIFI_FROOM_VS_VOTING) defaultId = SECTION_P1_WIFI_FROOM_VS_VOTING; //select section
            else if(defaultId == SECTION_P1_WIFI_FROM_FROOM_RACE || defaultId == SECTION_P2_WIFI_FROM_FROOM_RACE) {
                defaultId = SECTION_P2_WIFI_FROM_FROOM_RACE; //after room section
                SectionMgr::sInstance->sectionParams->localPlayerCount = 2;
            }
        }
    }
    return defaultId;
}

void OnDisconnectKO(SectionMgr* sectionMgr, SectionId id) {
    const System* system = System::sInstance;
    if(system->IsContext(PULSAR_MODE_KO)) id = system->koMgr->GetSectionAfterKO(id);
    sectionMgr->SetNextSection(id, 0);
}
kmCall(0x80651814, OnDisconnectKO);

PageId Mgr::KickPlayersOut(PageId defaultId) { //only called if KOMode

    PageId ret = defaultId;
    const System* system = System::sInstance;

    Mgr* mgr = system->koMgr;
    RacedataScenario& scenario = Racedata::sInstance->racesScenario;
    const bool isMainOut = mgr->IsKOdPlayerId(scenario.settings.hudPlayerIds[0]);
    if(system->nonTTGhostPlayersCount > 2) {
        if(scenario.localPlayerCount == 1) {
            const RKNet::Controller* controller = RKNet::Controller::sInstance;
            const RKNet::ControllerSub& sub = controller->subs[controller->currentSub];
            if(isMainOut) {
                if(sub.localAid == sub.hostAid) mgr->isSpectating = true; //force the host to spectate, they should not be allowed to quit
                else ret = static_cast<PageId>(RaceEndPage::id);
            }
        }
        else {
            const bool isGuestOut = mgr->IsKOdPlayerId(scenario.settings.hudPlayerIds[1]);
            if(isMainOut != isGuestOut) SectionMgr::sInstance->sectionParams->localPlayerCount = 1;
            if(isMainOut && !isGuestOut) {
                memcpy(&mgr->stats[0], &mgr->stats[1], sizeof(Mgr::Stats));
            }
            else if(isMainOut && isGuestOut) ret = static_cast<PageId>(RaceEndPage::id);

        }
    }
    return ret;
}

void Mgr::SwapControllersAndUI() {

    //Swap the controllers
    Input::Manager* input = Input::Manager::sInstance;

    char mainController[sizeof(Input::RealControllerHolder)];
    memcpy(&mainController, &input->realControllerHolders[0], sizeof(Input::RealControllerHolder));
    memcpy(&input->realControllerHolders[0], &input->realControllerHolders[1], sizeof(Input::RealControllerHolder));
    memcpy(&input->realControllerHolders[1], &mainController, sizeof(Input::RealControllerHolder));

    SectionMgr* sectionMgr = SectionMgr::sInstance;
    SectionPad& pad = sectionMgr->pad;
    PadInfo& main = pad.padInfos[0];
    PadInfo& guest = pad.padInfos[1];
    u32 old = main.controllerID;
    u32 oldg = guest.controllerID;
    main.controllerID = oldg;
    guest.controllerID = old;
    main.controllerIDActive = main.controllerID;
    guest.controllerIDActive = guest.controllerID;

    //Swap the characters on the UI
    SectionParams* params = sectionMgr->sectionParams;
    CharacterId mainChar = params->characters[0];
    KartId mainKart = params->karts[0];
    params->characters[0] = params->characters[1];
    params->karts[0] = params->karts[1];
    params->characters[1] = mainChar;
    params->karts[1] = mainKart;
    memcpy(&params->combos[0], &params->combos[1], sizeof(PlayerCombo));
    this->hasSwapped = !this->hasSwapped;
}
}//namespace KO
}//namespace Pulsar