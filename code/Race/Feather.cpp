#include <game/Item/ItemManager.hpp>
#include <game/Item/Obj/Gesso.hpp>
#include <game/Visual/Model/CourseModel.hpp>
#include <game/Visual/Model/ModelDirector.hpp>
#include <game/Visual/Model/KartModelsMgr.hpp>
#include <game/Network/network.hpp>
#include <Pulsar.hpp>
//Credit CLF78 and Stebler, this is mostly a port of their version with slightly different hooks and proper arguments naming since this is C++
void UseFeather(ItemPlayer *itemPlayer) {
    KartPointers *pointers = itemPlayer->kartPointers;
    pointers->kartMovement->specialFloor |= 0x4; //JumpPad

    KartStatus *status = pointers->kartStatus; //Hijacking bitfield1 14th bit to create a feather state
    u32 type = 0x7;
    if ((status->bitfield1 & 0x4000) != 0) type = 0x2; //if already in a feather, lower vertical velocity (30.0f instead of 50.0 for type 7)
    status->jumpPadType = type;
    status->trickableTimer = 0x4;

    itemPlayer->inventory.RemoveItems(1);

    if (KartModelsMgr::isOnlineRace && itemPlayer->isRemote) AddEVENTEntry(0xA, itemPlayer->id);
}

void UseBlooperOrFeather(ItemPlayer *itemPlayer) {
    if (Pulsar::sInstance->HasFeather()) UseFeather(itemPlayer);
    else itemPlayer->UseBlooper();
};
kmWritePointer(0x808A5894, UseBlooperOrFeather);
//kmBranch(0x807a81b4, UseBlooperOrFeather); //replaces UseBlooper



void ReplaceBlooperUseOtherPlayers(GessoMgr *gessoMgr, u8 id) {
    if (Pulsar::sInstance->HasFeather()) {
        UseFeather(&ItemManager::sInstance->players[id]);
    }
    else gessoMgr->DeployBlooper(id);

}
kmCall(0x80796d8c, ReplaceBlooperUseOtherPlayers); //replaces the small blooper model when someone uses a blooper with a feather use




kmWrite32(0x805b68d8, 0x7DE97B78); //mr r9, r15 to get playercollision
bool ConditionalIgnoreInvisibleWalls(float radius, CourseModel *model, Vec3 *position, Vec3 *lastPosition, KCLTypesBIT acceptedFlags, UnkType *normalsInfo, KCLTypeHolder &kclFlags, KartCollision *collision) {
    if (Pulsar::sInstance->HasFeather()) {
        KartStatus *status = collision->base.pointers->kartStatus;
        if (status->bitfield0 & 0x40000000 && status->jumpPadType == 0x7) acceptedFlags = (KCLTypesBIT)(acceptedFlags & ~(1 << KCL_INVISIBLE_WALL));
        //to remove invisible walls from the list of flags checked, these walls at flag 0xD and 2^0xD = 0x2000*
    }
    return model->ProcessCollision(radius, position, lastPosition, acceptedFlags, normalsInfo, kclFlags, 0);
}
kmCall(0x805b68dc, ConditionalIgnoreInvisibleWalls);

u8 ConditionalFastFallingBody(KartSub *sub) {
    if (Pulsar::sInstance->HasFeather()) {
        KartPhysicsHolder *physics = sub->base.GetKartPhysicsHolder();
        KartStatus *status = sub->base.pointers->kartStatus;
        if (status->bitfield0 & 0x40000000 && status->jumpPadType == 0x7 && status->airtime >= 2 && (!status->bool_0x97 || status->airtime > 19)) {
            ControllerHolder *controllerHolder = sub->base.GetControllerHolder();
            float input = controllerHolder->inputStates[0].stickY <= 0.0f ? 0.0f : (controllerHolder->inputStates[0].stickY + controllerHolder->inputStates[0].stickY);
            physics->kartPhysics->gravity -= input * 0.39f;
        }
    }
    return sub->base.GetPlayerIdx();
}
kmCall(0x805967ac, ConditionalFastFallingBody);


void ConditionalFastFallingWheels(float unk_float, WheelPhysicsHolder *wheelPhysicsHolder, Vec3 *gravityVector, Mtx34 *wheelMat) {
    if (Pulsar::sInstance->HasFeather()) {
        KartStatus *status = wheelPhysicsHolder->base.pointers->kartStatus;
        if (status->bitfield0 & 0x40000000 && status->jumpPadType == 0x7) {
            if (status->airtime == 0) status->bool_0x97 = ((status->bitfield0 & 0x80) != 0) ? true : false;
            else if (status->airtime >= 2 && (!status->bool_0x97 || status->airtime > 19)) {
                ControllerHolder *controllerHolder = wheelPhysicsHolder->base.GetControllerHolder();
                float input = controllerHolder->inputStates[0].stickY <= 0.0f ? 0.0f : (controllerHolder->inputStates[0].stickY + controllerHolder->inputStates[0].stickY);
                gravityVector->y -= input * 0.39f;
            }
        }
    }
    wheelPhysicsHolder->Update(unk_float, gravityVector, wheelMat);
}
kmCall(0x805973b4, ConditionalFastFallingWheels);


s32 HandleGroundFeatherCollision(KartCollision *collision) {
    if (Pulsar::sInstance->HasFeather()) {
        ItemPlayer *itemPlayer = &ItemManager::sInstance->players[collision->base.GetPlayerIdx()];
        itemPlayer->inventory.currentItemCount += 1;
        UseFeather(itemPlayer);
    }
    return -1;
}
kmWritePointer(0x808b54e8, HandleGroundFeatherCollision);

u32 ConditionalBlooperTimer(u32 timer) {
    if (Pulsar::sInstance->HasFeather()) timer = 0;
    else timer--;
    return timer;
}
kmCall(0x807bba64, ConditionalBlooperTimer);

void ConditionalFeatherBRRES(g3d::ResFile &file, ArchiveSource type, const char *brresName) {
    if (Pulsar::sInstance->HasFeather()) brresName = "feather.brres";
    ModelDirector::BindBRRES(file, type, brresName);
}
kmCall(0x807a84c8, ConditionalFeatherBRRES);

void ConditionalNoBlooperAnimation(ModelDirector *mdl, u32 id, g3d::ResFile *brres, const char *name, AnmType type, bool hasBlend,
    const char *brasd, ArchiveSource source, u8 kartArchiveIdx) {
    if (!Pulsar::sInstance->HasFeather()) {
        mdl->LinkAnimation(id, brres, name, type, hasBlend, brasd, source, kartArchiveIdx);
    }
}
kmCall(0x807a854c, ConditionalNoBlooperAnimation);
kmCall(0x807a857c, ConditionalNoBlooperAnimation);
kmCall(0x807a85ac, ConditionalNoBlooperAnimation);


void ConditionalObjProperties(ItemObjProperties *dest, ItemObjProperties *src) {
    dest->CopyFromRel(src);
    if (Pulsar::sInstance->HasFeather()) {
        dest->limit = 4;
        dest->canFallOnTheGround = true;
        dest->canFallOnTheGround2 = true;
    }
}
kmCall(0x80790bc4, ConditionalObjProperties);


void ConditionalBRCTR(ControlLoader *loader, const char *folderName, const char *ctrName, const char *variant, const char **animNames) {
    if (Pulsar::sInstance->HasFeather()) ctrName = "item_window_PUL";
    loader->Load(folderName, ctrName, variant, animNames);
}
kmCall(0x807ef50c, ConditionalBRCTR);

//Kept as is because it's almost never used and that guarantees ghost sync
kmWrite32(0x808b5c24, 0x42AA0000); //increases min, max speed of jump pad type 0x7 as well as its vertical velocity
kmWrite32(0x808b5c28, 0x42AA0000);
kmWrite32(0x808b5c2C, 0x42960000);