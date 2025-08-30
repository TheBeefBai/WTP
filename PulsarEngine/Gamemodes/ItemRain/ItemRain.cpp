#include <kamek.hpp>
#include <MarioKartWii/Item/ItemManager.hpp>
#include <MarioKartWii/System/Identifiers.hpp>
#include <MarioKartWii/Driver/DriverManager.hpp>
#include <MarioKartWii/Race/RaceInfo/RaceInfo.hpp>
#include <MarioKartWii/RKNet/RKNetController.hpp>
#include <MarioKartWii/RKNet/ITEM.hpp>
#include <MarioKartWii/Item/Obj/ItemObj.hpp>
#include <MarioKartWii/Kart/KartPlayer.hpp>
#include <MarioKartWii/Kart/KartMovement.hpp>
#include <PulsarSystem.hpp>

namespace Pulsar {
namespace ItemRain {

static s32 sRaceInfoFrameCounter = 0;

static int ITEMS_PER_SPAWN = 1;
static int SPAWN_HEIGHT = 1500;
static int SPAWN_RADIUS = 8000;
static int MAX_ITEM_LIFETIME = 360;
static int DESPAWN_CHECK_INTERVAL = 2;

static int GetSpawnInterval(u8 playerCount) {
    if (playerCount <= 3) return 6;
    if (playerCount <= 6) return 12;
    if (playerCount <= 9) return 18;
    return 24;
}

static u32 GetRandom() {
    // Use frame counter and some bit manipulation for pseudo-randomness
    static u32 counter = 0;
    counter++;
    u32 value = (sRaceInfoFrameCounter * 1103515245 + counter * 12345) ^ (counter << 8);
    return value * 1664525 + 1013904223;
}

static int GetRandomRange(int min, int max) {
    return min + (GetRandom() % (max - min + 1));
}

static float GetRandomRange(float min, float max) {
    return min + ((GetRandom() & 0xFFFF) / 65535.0f) * (max - min);
}

static ItemObjId GetRandomItem() {
    struct ItemWeight {
        ItemObjId id;
        u32 weight;
    };
        
    static const ItemWeight weightedItems[] = {
        {OBJ_MUSHROOM, 20},
        {OBJ_GREEN_SHELL, 15},
        {OBJ_BANANA, 16},
        {OBJ_RED_SHELL, 5},
        {OBJ_FAKE_ITEM_BOX, 8},
        {OBJ_BOBOMB, 5},
        {OBJ_STAR, 7},
        {OBJ_BLUE_SHELL, 3},
        {OBJ_GOLDEN_MUSHROOM, 3},
        {OBJ_MEGA_MUSHROOM, 10},
        {OBJ_POW_BLOCK, 1},
        {OBJ_BULLET_BILL, 5},
        {OBJ_LIGHTNING, 2}
    };
        
    const u32 totalWeight = 100; // Sum of all weights
    u32 roll = GetRandom() % totalWeight;
    u32 cumulative = 0;
        
    for (u32 i = 0; i < sizeof(weightedItems) / sizeof(weightedItems[0]); i++) {
        cumulative += weightedItems[i].weight;
        if (roll < cumulative) {
            return weightedItems[i].id;
        }
    }
        
    return OBJ_MUSHROOM; // Fallback
}

void DespawnItems(bool checkDistance = false) {
    if (!Item::Manager::sInstance) return;
        
    u8 playerCount = 0;
    Vec3 playerPositions[12];
    if (checkDistance) {
        playerCount = Pulsar::System::sInstance ? Pulsar::System::sInstance->nonTTGhostPlayersCount : 0;
        for (int i = 0; i < playerCount && i < 12; i++) {
            playerPositions[i] = Item::Manager::sInstance->players[i].GetPosition();
        }
    }
        
    for (int i = 0; i < 0xF; i++) {
        Item::ObjHolder& holder = Item::Manager::sInstance->itemObjHolders[i];
        if (holder.itemObjId == OBJ_NONE || holder.bodyCount == 0 || holder.itemObjId == OBJ_THUNDER_CLOUD) continue;
            
        for (u32 j = 0; j < holder.capacity; j++) {
            Item::Obj* obj = holder.itemObj[j];
            if (!obj || (obj->bitfield74 & 0x1)) continue;
                
            bool shouldDespawn = obj->duration > MAX_ITEM_LIFETIME;
                
            if (checkDistance && obj->duration >= 900) {
                bool farFromAll = true;
                for (int k = 0; k < playerCount && k < 12 && farFromAll; k++) {
                    Vec3 diff;
                    diff.x = obj->position.x - playerPositions[k].x;
                    diff.y = obj->position.y - playerPositions[k].y;
                    diff.z = obj->position.z - playerPositions[k].z;
                    if (diff.x * diff.x + diff.y * diff.y + diff.z * diff.z < 225000000.0f) {
                        farFromAll = false;
                    }
                }
                shouldDespawn |= farFromAll;
            }
                
            if (shouldDespawn) {
                obj->DisappearDueToExcess(true);
            }
        }
    }
}

void SpawnItemRain() {
    const RacedataScenario& scenario = Racedata::sInstance->racesScenario;
    const GameMode mode = scenario.settings.gamemode;
    if (!Pulsar::System::sInstance->IsContext(PULSAR_GAMEMODEITEMRAIN)) return;
    if (Pulsar::System::sInstance->IsContext(PULSAR_MODE_OTT)) return;
    if (RKNet::Controller::sInstance->roomType != RKNet::ROOMTYPE_FROOM_HOST && RKNet::Controller::sInstance->roomType != RKNet::ROOMTYPE_FROOM_NONHOST && 
        RKNet::Controller::sInstance->roomType != RKNet::ROOMTYPE_NONE && RKNet::Controller::sInstance->roomType != RKNet::ROOMTYPE_VS_REGIONAL) return;
    if (mode == MODE_TIME_TRIAL) return;
    if (!Racedata::sInstance || !Raceinfo::sInstance || !Item::Manager::sInstance) return;
    if (!Raceinfo::sInstance->IsAtLeastStage(RACESTAGE_RACE)) return;

    sRaceInfoFrameCounter++;
        
    if ((sRaceInfoFrameCounter % DESPAWN_CHECK_INTERVAL) == 0) {
        DespawnItems();
    }
        
    if ((sRaceInfoFrameCounter % (DESPAWN_CHECK_INTERVAL * 2)) == 0) {
        DespawnItems(true);
    }
        
    u8 playerCount = Pulsar::System::sInstance->nonTTGhostPlayersCount;
    if (playerCount == 0) return;

    if ((sRaceInfoFrameCounter % GetSpawnInterval(playerCount)) != 0) return;

    Vec3 dummyDirection;
    dummyDirection.x = 0.0f;
    dummyDirection.y = 0.0f;
    dummyDirection.z = 0.0f;

    for (int i = 0; i < ITEMS_PER_SPAWN; i++) {
        for (int playerIdx = 0; playerIdx < playerCount; playerIdx++) {
            Item::Player& player = Item::Manager::sInstance->players[playerIdx];
            Vec3 playerPos = player.GetPosition();

            Vec3 forwardDir;
            if (player.kartPlayer) {
                forwardDir = player.kartPlayer->GetMovement().dir;
            } else {
                forwardDir.x = 0.0f;
                forwardDir.y = 0.0f;
                forwardDir.z = 1.0f;
            }
                
            Vec3 rightDir;
            rightDir.x = forwardDir.z;
            rightDir.y = 0.0f;
            rightDir.z = -forwardDir.x;

            float forward = GetRandomRange(1000.0f, 12000.0f);
            float side = GetRandomRange(-SPAWN_RADIUS, SPAWN_RADIUS);
                
            Vec3 spawnPos;
            spawnPos.x = playerPos.x + forwardDir.x * forward + rightDir.x * side;
            spawnPos.y = playerPos.y + SPAWN_HEIGHT;
            spawnPos.z = playerPos.z + forwardDir.z * forward + rightDir.z * side;

            ItemObjId selectedItem = GetRandomItem();
                
            // Try multiple spawn attempts to ensure items actually appear
            for (int attempt = 0; attempt < 3; attempt++) {
                if (attempt > 0) {
                    // Use different positions if first attempts fail
                    float newForward = GetRandomRange(1000.0f, 12000.0f);
                    float newSide = GetRandomRange(-SPAWN_RADIUS, SPAWN_RADIUS);
                    spawnPos.x = playerPos.x + forwardDir.x * newForward + rightDir.x * newSide;
                    spawnPos.z = playerPos.z + forwardDir.z * newForward + rightDir.z * newSide;
                }
                    
                Item::Manager::sInstance->CreateItemDirect(selectedItem, &spawnPos, &dummyDirection, playerIdx);
            }
        }
    }
}
RaceFrameHook ItemRainHook(SpawnItemRain);

//Item Spam Anti-Freeze [???]
asmFunc ItemSpamAntiFreeze() {
    ASM (
        loc_0x0:
        lbz       r12, 0x1C(r27);
        add       r12, r30, r12;
        cmpwi     r12, 0xE0;
        blt+      loc_0x18;
        li        r0, 0;
        stb       r0, 0x19(r27);
        
        loc_0x18:
        lbz       r0, 0x19(r27);
    )
}
kmCall(0x8065BBD4, ItemSpamAntiFreeze);

} // namespace ItemRain
} // namespace Pulsar