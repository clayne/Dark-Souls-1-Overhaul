#include "Rollback.h"
#include "SP/memory.h"
#include "SP/memory/injection/asm/x64.h"
#include "DarkSoulsOverhaulMod.h"
#include "MainLoop.h"
#include "InputUtil.h"

void* malloc_(size_t size)
{
    void* out = malloc(size);
    if (out == NULL)
    {
        FATALERROR("Malloc failure. Unable to allocate %d bytes.", size);
    }
    memset(out, 0, size);
    return out;
}

extern "C" {
    uint64_t sendType1NetMessage_injection_return;
    void sendType1NetMessage_injection();
    uint64_t recvType1NetMessage_injection_return;
    void recvType1NetMessage_injection();
}

PlayerIns* Rollback::saved_playerins = NULL;

bool Rollback::bsave = false;
bool Rollback::bload = false;
bool rollback_test(void* unused)
{
    if (Rollback::bsave)
    {
        Rollback::save();
        Rollback::bsave = false;
    }

    if (Rollback::bload)
    {
        Rollback::load();
        Rollback::bload = false;
    }

    return true;
}

void Rollback::start()
{
    ConsoleWrite("Rollback...");
    uint8_t *write_address;

    // normally, type18 packet receipt is the only way to do damage to another player
    // the client does see the other attacking player as a normal npc, and is capable of applying damage the normal way
    // but explicitly doesn't if the other player is a PC
    // disable that throw away check and just return 0 instead
    write_address = (uint8_t*)(Rollback::disableType18PacketEnforcement + Game::ds1_base);
    uint8_t disableType18PacketEnforcement_patch[] = { 0x48, 0x31, 0xc0, 0x90, 0x90, 0x90 }; //xor    rax,rax | nop | nop | nop
    sp::mem::patch_bytes(write_address, disableType18PacketEnforcement_patch, sizeof(disableType18PacketEnforcement_patch));

    // see the above, disable the sending of the type18 packet, since we now compute it client-side
    write_address = (uint8_t*)(Rollback::disableType18PacketSending + Game::ds1_base);
    uint8_t disableType18PacketSending_patch[] = { 0x90, 0x90, 0x90, 0x90, 0x90 }; //nop x5
    sp::mem::patch_bytes(write_address, disableType18PacketSending_patch, sizeof(disableType18PacketSending_patch));

    //init out copy of the playerins struct, for saving/restoring with rollback
    Rollback::saved_playerins = init_PlayerIns();

    //TEST save/restore with a hotkey
    MainLoop::setup_mainloop_callback(rollback_test, NULL, "rollback_test");
}

void Rollback::save()
{
    auto player_o = Game::get_PlayerIns();
    if (!player_o.has_value())
    {
        return;
    }
    PlayerIns* player = (PlayerIns*)player_o.value();

    //we pre-allocate a static playerins on boot, so we can assume all pointers are set up
    copy_PlayerIns(Rollback::saved_playerins, player, false);
}

void Rollback::load()
{
    auto player_o = Game::get_PlayerIns();
    if (!player_o.has_value())
    {
        return;
    }
    PlayerIns* player = (PlayerIns*)player_o.value();

    copy_PlayerIns(player, Rollback::saved_playerins, true);
}

void copy_PlayerIns(PlayerIns* to, PlayerIns* from, bool to_game)
{
    copy_ChrIns(&to->chrins, &from->chrins, to_game);
    copy_PlayerGameData(to->playergamedata, from->playergamedata);
    copy_RingEquipCtrl(to->ringequipctrl, from->ringequipctrl, to_game);
    copy_WeaponEquipCtrl(to->weaponequipctrl, from->weaponequipctrl, to_game);
    copy_ProEquipCtrl(to->proequipctrl, from->proequipctrl, to_game);
    to->curSelectedMagicId = from->curSelectedMagicId;
    to->curUsedItem = from->curUsedItem;
    to->itemId = from->itemId;
    to->override_equipped_magicId = from->override_equipped_magicId;
    copy_ChrAsm(to->chrasm, from->chrasm);
}

PlayerIns* init_PlayerIns()
{
    PlayerIns* local_PlayerIns = (PlayerIns*)malloc_(sizeof(PlayerIns));

    ChrIns* pChrIns = init_ChrIns();
    local_PlayerIns->chrins = *pChrIns;
    free(pChrIns);
    local_PlayerIns->playergamedata = init_PlayerGameData();
    local_PlayerIns->ringequipctrl = init_RingEquipCtrl();
    local_PlayerIns->weaponequipctrl = init_WeaponEquipCtrl();
    local_PlayerIns->proequipctrl = init_ProEquipCtrl();
    local_PlayerIns->chrasm = init_ChrAsm();

    return local_PlayerIns;
}

void copy_ChrAsm(ChrAsm* to, ChrAsm* from)
{
    to->equipped_weapon_style = from->equipped_weapon_style;
    to->l_hand_equipped_index = from->equipped_weapon_style;
    to->r_hand_equipped_index = from->r_hand_equipped_index;
    memcpy(to->equip_items, from->equip_items, sizeof(to->equip_items));
}

ChrAsm* init_ChrAsm()
{
    ChrAsm* local_ChrAsm = (ChrAsm*)malloc_(sizeof(ChrAsm));
    return local_ChrAsm;
}

void copy_ProEquipCtrl(ProEquipCtrl* to, ProEquipCtrl* from, bool to_game)
{
    copy_SpecialEffect(to->spEffectList, from->spEffectList, to_game);
    //there should always be 5 armors (4 equip and a hair)
    if (from->array_len != 5)
    {
        FATALERROR("Got %d for ProEquipCtrl->array_len, should be 5", from->array_len);
    }
    to->array_len = 5;
    to->equipped_armors_ids[0] = from->equipped_armors_ids[0];
    to->equipped_armors_ids[1] = from->equipped_armors_ids[1];
    to->equipped_armors_ids[2] = from->equipped_armors_ids[2];
    to->equipped_armors_ids[3] = from->equipped_armors_ids[3];
    to->equipped_armors_ids[4] = from->equipped_armors_ids[4];
}

ProEquipCtrl* init_ProEquipCtrl()
{
    ProEquipCtrl* local_ProEquipCtrl = (ProEquipCtrl*)malloc_(sizeof(ProEquipCtrl));
    uint32_t* local_ProEquipCtrl_equipped_armors_ids = (uint32_t*)malloc_(sizeof(uint32_t)*5);

    local_ProEquipCtrl->spEffectList = init_SpecialEffect();
    local_ProEquipCtrl->equipped_armors_ids = local_ProEquipCtrl_equipped_armors_ids;

    return local_ProEquipCtrl;
}

void copy_WeaponEquipCtrl(WeaponEquipCtrl* to, WeaponEquipCtrl* from, bool to_game)
{
    copy_SpecialEffect(to->spEffectList, from->spEffectList, to_game);
    //there should always be 2 weapons
    if (from->array_len != 2)
    {
        FATALERROR("Got %d for WeaponEquipCtrl->array_len", from->array_len);
    }
    to->array_len = 2;
    to->equipped_weapons_ids[0] = from->equipped_weapons_ids[0];
    to->equipped_weapons_ids[1] = from->equipped_weapons_ids[1];
}

WeaponEquipCtrl* init_WeaponEquipCtrl()
{
    WeaponEquipCtrl* local_WeaponEquipCtrl = (WeaponEquipCtrl*)malloc_(sizeof(WeaponEquipCtrl));
    uint32_t* local_WeaponEquipCtrl_equipped_weapons_ids = (uint32_t*)malloc_(sizeof(uint32_t)*2);

    local_WeaponEquipCtrl->spEffectList = init_SpecialEffect();
    local_WeaponEquipCtrl->equipped_weapons_ids = local_WeaponEquipCtrl_equipped_weapons_ids;

    return local_WeaponEquipCtrl;
}

void copy_RingEquipCtrl(RingEquipCtrl* to, RingEquipCtrl* from, bool to_game)
{
    copy_SpecialEffect(to->spEffectList, from->spEffectList, to_game);
    //there should always be 2 rings
    if (from->array_len != 2)
    {
        FATALERROR("Got %d for RingEquipCtrl->array_len", from->array_len);
    }
    to->array_len = 2;
    to->equipped_rings_ids[0] = from->equipped_rings_ids[0];
    to->equipped_rings_ids[1] = from->equipped_rings_ids[1];
}

RingEquipCtrl* init_RingEquipCtrl()
{
    RingEquipCtrl* local_RingEquipCtrl = (RingEquipCtrl*)malloc_(sizeof(RingEquipCtrl));
    uint32_t* local_RingEquipCtrl_equipped_rings_ids = (uint32_t*)malloc_(sizeof(uint32_t)*2);

    local_RingEquipCtrl->spEffectList = init_SpecialEffect();
    local_RingEquipCtrl->equipped_rings_ids = local_RingEquipCtrl_equipped_rings_ids;

    return local_RingEquipCtrl;
}

void copy_PlayerGameData(PlayerGameData* to, PlayerGameData* from)
{
    copy_PlayerGameData_AttributeInfo(&to->attribs, &from->attribs);
    copy_EquipGameData(&to->equipGameData, &from->equipGameData);
    copy_PlayerGameData_ChrProperties(&to->ChrProperties, &from->ChrProperties);
}

PlayerGameData* init_PlayerGameData()
{
    PlayerGameData* local_PlayerGameData = (PlayerGameData*)malloc_(sizeof(PlayerGameData));

    EquipGameData* pEquipGameData = init_EquipGameData();
    local_PlayerGameData->equipGameData = *pEquipGameData;
    free(pEquipGameData);

    return local_PlayerGameData;
}

void copy_PlayerGameData_ChrProperties(PlayerGameData_ChrProperties* to, PlayerGameData_ChrProperties* from)
{
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
}

void copy_EquipGameData(EquipGameData* to, EquipGameData* from)
{
    copy_EquipMagicData(to->equipMagicData, from->equipMagicData);
    copy_EquipItemData(&to->equippedItemsInQuickbar, &from->equippedItemsInQuickbar);
}

EquipGameData* init_EquipGameData()
{
    EquipGameData* local_EquipGameData = (EquipGameData*)malloc_(sizeof(EquipGameData));

    local_EquipGameData->equipMagicData = init_EquipMagicData();

    return local_EquipGameData;
}

void copy_EquipItemData(EquipItemData* to, EquipItemData* from)
{
    memcpy(to->quickbar, from->quickbar, sizeof(to->quickbar));
}

void copy_EquipMagicData(EquipMagicData* to, EquipMagicData* from)
{
    memcpy(to->equippedMagicList, from->equippedMagicList, sizeof(to->equippedMagicList));
    to->curSelectedMagicSlot = from->curSelectedMagicSlot;
}

EquipMagicData* init_EquipMagicData()
{
    EquipMagicData* local_EquipMagicData = (EquipMagicData*)malloc_(sizeof(EquipMagicData));
    return local_EquipMagicData;
}

void copy_PlayerGameData_AttributeInfo(PlayerGameData_AttributeInfo* to, PlayerGameData_AttributeInfo* from)
{
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
}

void copy_ChrIns(ChrIns* to, ChrIns* from, bool to_game)
{
    copy_PlayerCtrl(to->playerCtrl, from->playerCtrl);
    copy_PadManipulator(to->padManipulator, from->padManipulator);
    to->CharaInitParamID = from->CharaInitParamID;
    to->lowerThrowAnim = from->lowerThrowAnim;
    to->upperThrowAnim = from->upperThrowAnim;
    to->curSelectedMagicId = from->curSelectedMagicId;
    to->curUsedItem = from->curUsedItem;
    copy_SpecialEffect(to->specialEffects, from->specialEffects, to_game);
    copy_QwcSpEffectEquipCtrl(to->qwcSpEffectEquipCtrl, from->qwcSpEffectEquipCtrl);
    to->curHp = from->curHp;
    to->maxHp = from->maxHp;
    to->curSp = from->curSp;
    to->maxSp = from->maxSp;
    to->damage_taken_scalar = from->damage_taken_scalar;
    to->PoisonResist = from->PoisonResist;
    to->ToxicResist = from->ToxicResist;
    to->BleedResist = from->BleedResist;
    to->CurseResist = from->CurseResist;
    to->resistPoisonTotal = from->resistPoisonTotal;
    to->resistPlagueTotal = from->resistPlagueTotal;
    to->resistBleedingTotal = from->resistBleedingTotal;
    to->resistCurseTotal = from->resistCurseTotal;
    copy_EntityThrowAnimationStatus(to->throw_animation_info, from->throw_animation_info);
}

ChrIns* init_ChrIns()
{
    ChrIns* local_ChrIns = (ChrIns*)malloc_(sizeof(ChrIns));

    local_ChrIns->playerCtrl = init_PlayerCtrl();
    local_ChrIns->padManipulator = init_PadManipulator();
    local_ChrIns->specialEffects = init_SpecialEffect();
    local_ChrIns->qwcSpEffectEquipCtrl = init_QwcSpEffectEquipCtrl();
    local_ChrIns->throw_animation_info = init_EntityThrowAnimationStatus();

    return local_ChrIns;
}

void copy_EntityThrowAnimationStatus(EntityThrowAnimationStatus* to, EntityThrowAnimationStatus* from)
{
    to->throw_paramdef = from->throw_paramdef;
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
    memcpy(to->data_1, from->data_1, sizeof(to->data_1));
}

EntityThrowAnimationStatus* init_EntityThrowAnimationStatus()
{
    EntityThrowAnimationStatus* local_EntityThrowAnimationStatus = (EntityThrowAnimationStatus*)malloc_(sizeof(EntityThrowAnimationStatus));
    return local_EntityThrowAnimationStatus;
}

void copy_QwcSpEffectEquipCtrl(QwcSpEffectEquipCtrl* to, QwcSpEffectEquipCtrl* from)
{
    //we allow up to a max of 64 speffects
    if (from->arry_len > 64)
    {
        FATALERROR("Got %d number of speffects for QwcSpEffectEquipCtrl->arry. Only support a max of 64.");
    }
    to->arry_len = from->arry_len;
    for (size_t i = 0; i < from->arry_len; i++)
    {
        to->arry[i] = from->arry[i];
    }
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
}

QwcSpEffectEquipCtrl* init_QwcSpEffectEquipCtrl()
{
    QwcSpEffectEquipCtrl* local_QwcSpEffectEquipCtrl = (QwcSpEffectEquipCtrl*)malloc_(sizeof(QwcSpEffectEquipCtrl));
    uint32_t* local_QwcSpEffectEquipCtrl_arry = (uint32_t*)malloc_(sizeof(uint32_t)*64);

    local_QwcSpEffectEquipCtrl->arry = local_QwcSpEffectEquipCtrl_arry;

    return local_QwcSpEffectEquipCtrl;
}

void copy_SpecialEffect(SpecialEffect* to, SpecialEffect* from, bool to_game)
{
    copy_SpecialEffect_Info(to->specialEffect_Info, from->specialEffect_Info, to_game);
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
    memcpy(to->data_1, from->data_1, sizeof(to->data_1));
    memcpy(to->data_2, from->data_2, sizeof(to->data_2));
}

SpecialEffect* init_SpecialEffect()
{
    SpecialEffect* local_SpecialEffect = (SpecialEffect*)malloc_(sizeof(SpecialEffect));

    local_SpecialEffect->specialEffect_Info = init_SpecialEffect_Info();

    return local_SpecialEffect;
}

static const size_t max_preallocated_SpecialEffect_Info = 64;

void copy_SpecialEffect_Info(SpecialEffect_Info* to, SpecialEffect_Info* from_, bool to_game)
{
    SpecialEffect_Info* from = from_;

    if (!to_game)
    {
        size_t to_index = 0;
        while (from)
        {
            if (to_index >= max_preallocated_SpecialEffect_Info)
            {
                ConsoleWrite("Unable to recursivly copy SpecialEffect_Info from the game. Out of space.");
                break;
            }
            memcpy(to->data_0, from->data_0, sizeof(to->data_0));
            to->paramRowBytes = from->paramRowBytes;

            from = from->next;
            to += sizeof(SpecialEffect_Info);
            to_index += 1;
        }
    }
    else
    {
        size_t from_index = 0;
        while (to)
        {
            if (from_index >= max_preallocated_SpecialEffect_Info)
            {
                ConsoleWrite("Unable to recursivly copy SpecialEffect_Info into the game. Out of space.");
                break;
            }
            //check if this entry is an actual saved value
            if (from->paramRowBytes == 0)
            {
                //if not and the game has extra links still to go, we need to free them and set the next on the last used on to null
                SpecialEffect_Info* entry_to_free = to->next;
                to->next = NULL;
                while (entry_to_free)
                {
                    SpecialEffect_Info* next = entry_to_free->next;
                    Game::game_free(entry_to_free, sizeof(SpecialEffect_Info));
                    entry_to_free = next;
                }
                break;
            }
            else
            {
                memcpy(to->data_0, from->data_0, sizeof(to->data_0));
                to->paramRowBytes = from->paramRowBytes;
            }

            from += sizeof(SpecialEffect_Info);
            to = to->next;
            from_index += 1;
        }

        //clear the local speffect infos after we're done so we don't have stale data copied back later
        memset(from_, 0, sizeof(SpecialEffect_Info)*max_preallocated_SpecialEffect_Info);
    }
}

SpecialEffect_Info* init_SpecialEffect_Info()
{
    //this is a linked list, so pre-allocate a max of 64 in the list for speffects
    SpecialEffect_Info* local_SpecialEffect_Info = (SpecialEffect_Info*)malloc_(sizeof(SpecialEffect_Info)*max_preallocated_SpecialEffect_Info);

    return local_SpecialEffect_Info;
}

void copy_PadManipulator(PadManipulator* to, PadManipulator* from)
{
    copy_ChrManipulator(&to->chrManipulator, &from->chrManipulator);
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
    memcpy(to->data_1, from->data_1, sizeof(to->data_1));
}

PadManipulator* init_PadManipulator()
{
    PadManipulator* local_PadManipulator = (PadManipulator*)malloc_(sizeof(PadManipulator));
    return local_PadManipulator;
}

void copy_NetworkManipulator(NetworkManipulator* to, NetworkManipulator* from)
{
    //TODO
}

void copy_ChrManipulator(ChrManipulator* to, ChrManipulator* from)
{
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
}

void copy_PlayerCtrl(PlayerCtrl* to, PlayerCtrl* from)
{
    copy_ChrCtrl(&to->chrCtrl, &from->chrCtrl);
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
    copy_TurnAnim(to->turnAnim, from->turnAnim);
    copy_ArrowTurnAnim(to->arrowTurnAnim, from->arrowTurnAnim);
    memcpy(to->data_1, from->data_1, sizeof(to->data_1));
    memcpy(to->data_2, from->data_2, sizeof(to->data_2));
}

PlayerCtrl* init_PlayerCtrl()
{
    PlayerCtrl* local_PlayerCtrl = (PlayerCtrl*)malloc_(sizeof(PlayerCtrl));

    ChrCtrl* pChrCtrl = init_ChrCtrl();
    local_PlayerCtrl->chrCtrl = *pChrCtrl;
    free(pChrCtrl);
    local_PlayerCtrl->turnAnim = init_TurnAnim();
    local_PlayerCtrl->arrowTurnAnim = init_ArrowTurnAnim();

    return local_PlayerCtrl;
}

void copy_ArrowTurnAnim(ArrowTurnAnim* to, ArrowTurnAnim* from)
{
    copy_TurnAnim(&to->turnAnim, &from->turnAnim);
    copy_SpinJoint(to->joint_spine_2, from->joint_spine_2);
    copy_SpinJoint(to->joint_spine1_2, from->joint_spine1_2);
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
}

ArrowTurnAnim* init_ArrowTurnAnim()
{
    ArrowTurnAnim* local_ArrowTurnAnim = (ArrowTurnAnim*)malloc_(sizeof(ArrowTurnAnim));

    TurnAnim* pTurnAnim = init_TurnAnim();
    local_ArrowTurnAnim->turnAnim = *pTurnAnim;
    free(pTurnAnim);
    local_ArrowTurnAnim->joint_spine_2 = init_SpinJoint();
    local_ArrowTurnAnim->joint_spine1_2 = init_SpinJoint();

    return local_ArrowTurnAnim;
}

void copy_SpinJoint(SpinJoint* to, SpinJoint* from)
{
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
    memcpy(to->data_1, from->data_1, sizeof(to->data_1));
    memcpy(to->data_2, from->data_2, sizeof(to->data_2));
}

SpinJoint* init_SpinJoint()
{
    SpinJoint* local_SpinJoint = (SpinJoint*)malloc_(sizeof(SpinJoint));
    return local_SpinJoint;
}

void copy_TurnAnim(TurnAnim* to, TurnAnim* from)
{
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
    memcpy(to->data_1, from->data_1, sizeof(to->data_1));
    copy_SpinJoint(to->joint_UpperRoot, from->joint_UpperRoot);
    copy_SpinJoint(to->joint_LowerRoot, from->joint_LowerRoot);
    copy_SpinJoint(to->joint_spine1_1, from->joint_spine1_1);
    copy_SpinJoint(to->joint_spine_1, from->joint_spine_1);
    copy_SpinJoint(to->joint_master, from->joint_master);
    memcpy(to->data_2, from->data_2, sizeof(to->data_2));
    memcpy(to->data_3, from->data_3, sizeof(to->data_3));
}

TurnAnim* init_TurnAnim()
{
    TurnAnim* local_TurnAnim = (TurnAnim*)malloc_(sizeof(TurnAnim));

    local_TurnAnim->joint_UpperRoot = init_SpinJoint();
    local_TurnAnim->joint_LowerRoot = init_SpinJoint();
    local_TurnAnim->joint_spine1_1 = init_SpinJoint();
    local_TurnAnim->joint_spine_1 = init_SpinJoint();
    local_TurnAnim->joint_master = init_SpinJoint();

    return local_TurnAnim;
}

void copy_ChrCtrl(ChrCtrl* to, ChrCtrl* from)
{
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
    copy_ChrCtrl_AnimationQueue(to->animationQueue, from->animationQueue);
    copy_AnimationMediator(to->animationMediator, from->animationMediator);
    copy_HavokChara(to->havokChara, from->havokChara);
    copy_ActionCtrl(to->actionctrl, from->actionctrl);
    memcpy(to->data_1, from->data_1, sizeof(to->data_1));
    memcpy(to->data_2, from->data_2, sizeof(to->data_2));
    copy_WalkAnim_Twist(to->walkAnim_Twist, from->walkAnim_Twist);
    memcpy(to->data_3, from->data_3, sizeof(to->data_3));
    memcpy(to->data_4, from->data_4, sizeof(to->data_4));
}

ChrCtrl* init_ChrCtrl()
{
    ChrCtrl* local_ChrCtrl = (ChrCtrl*)malloc_(sizeof(ChrCtrl));

    local_ChrCtrl->animationQueue = init_ChrCtrl_AnimationQueue();
    local_ChrCtrl->animationMediator = init_AnimationMediator();
    local_ChrCtrl->havokChara = init_HavokChara();
    local_ChrCtrl->actionctrl = init_ActionCtrl();
    local_ChrCtrl->walkAnim_Twist = init_WalkAnim_Twist();

    return local_ChrCtrl;
}

void copy_WalkAnim_Twist(WalkAnim_Twist* to, WalkAnim_Twist* from)
{
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
    memcpy(to->data_1, from->data_1, sizeof(to->data_1));
    memcpy(to->data_2, from->data_2, sizeof(to->data_2));
    copy_SpinJoint(to->Upper_Root_Joint, from->Upper_Root_Joint);
    copy_SpinJoint(to->master_joint, from->master_joint);
    copy_SpinJoint(to->neck_joint, from->neck_joint);
    memcpy(to->data_3, from->data_3, sizeof(to->data_3));
    copy_WalkAnim_Twist_Field0x228Elem(&to->walkAnim_Twist_Field0x228Elem, &from->walkAnim_Twist_Field0x228Elem);
    memcpy(to->data_4, from->data_4, sizeof(to->data_4));
}

WalkAnim_Twist* init_WalkAnim_Twist()
{
    WalkAnim_Twist* local_WalkAnim_Twist = (WalkAnim_Twist*)malloc_(sizeof(WalkAnim_Twist));

    local_WalkAnim_Twist->Upper_Root_Joint = init_SpinJoint();
    local_WalkAnim_Twist->master_joint = init_SpinJoint();
    local_WalkAnim_Twist->neck_joint = init_SpinJoint();
    WalkAnim_Twist_Field0x228Elem* pWalkAnim_Twist_Field0x228Elem = init_WalkAnim_Twist_Field0x228Elem();
    local_WalkAnim_Twist->walkAnim_Twist_Field0x228Elem = *pWalkAnim_Twist_Field0x228Elem;
    free(pWalkAnim_Twist_Field0x228Elem);

    return local_WalkAnim_Twist;
}

void copy_WalkAnim_Twist_Field0x228Elem(WalkAnim_Twist_Field0x228Elem* to, WalkAnim_Twist_Field0x228Elem* from)
{
    //TODO
}

WalkAnim_Twist_Field0x228Elem* init_WalkAnim_Twist_Field0x228Elem()
{
    WalkAnim_Twist_Field0x228Elem* local_WalkAnim_Twist_Field0x228Elem = (WalkAnim_Twist_Field0x228Elem*)malloc_(sizeof(WalkAnim_Twist_Field0x228Elem));
    //TODO
    return local_WalkAnim_Twist_Field0x228Elem;
}

void copy_ActionCtrl(ActionCtrl* to, ActionCtrl* from)
{
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
    copy_ActionCtrl_0x30Substruct(&to->passive_state, &from->passive_state);
    copy_ActionCtrl_0x30Substruct(&to->active_state, &from->active_state);
    memcpy(to->data_1, from->data_1, sizeof(to->data_1));
    memcpy(to->data_2, from->data_2, sizeof(to->data_2));
}

ActionCtrl* init_ActionCtrl()
{
    ActionCtrl* local_ActionCtrl = (ActionCtrl*)malloc_(sizeof(ActionCtrl));

    local_ActionCtrl->passive_state.EzStateMachineImpl = init_EzState_detail_EzStateMachineImpl();
    local_ActionCtrl->active_state.EzStateMachineImpl = init_EzState_detail_EzStateMachineImpl();

    return local_ActionCtrl;
}

void copy_ActionCtrl_0x30Substruct(ActionCtrl_0x30Substruct* to, ActionCtrl_0x30Substruct* from)
{
    copy_EzState_detail_EzStateMachineImpl(to->EzStateMachineImpl, from->EzStateMachineImpl);
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
}

void copy_EzState_detail_EzStateMachineImpl(EzState_detail_EzStateMachineImpl* to, EzState_detail_EzStateMachineImpl* from)
{
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
    memcpy(to->data_1, from->data_1, sizeof(to->data_1));
    memcpy(to->data_2, from->data_2, sizeof(to->data_2));
}

EzState_detail_EzStateMachineImpl* init_EzState_detail_EzStateMachineImpl()
{
    EzState_detail_EzStateMachineImpl* local_EzState_detail_EzStateMachineImpl = (EzState_detail_EzStateMachineImpl*)malloc_(sizeof(EzState_detail_EzStateMachineImpl));
    return local_EzState_detail_EzStateMachineImpl;
}

void copy_HavokChara(HavokChara* to, HavokChara* from)
{
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
    copy_hkpCharacterProxy(to->char_proxy, from->char_proxy);
    memcpy(to->data_1, from->data_1, sizeof(to->data_1));
}

HavokChara* init_HavokChara()
{
    HavokChara* local_HavokChara = (HavokChara*)malloc_(sizeof(HavokChara));

    local_HavokChara->char_proxy = init_hkpCharacterProxy();

    return local_HavokChara;
}

void copy_hkpCharacterProxy(hkpCharacterProxy* to, hkpCharacterProxy* from)
{
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
}

hkpCharacterProxy* init_hkpCharacterProxy()
{
    hkpCharacterProxy* local_hkpCharacterProxy = (hkpCharacterProxy*)malloc_(sizeof(hkpCharacterProxy));
    return local_hkpCharacterProxy;
}

//use this to check if we have already saved a linked_animation. It maps from the address of a from->linked_animation to our saved to
static std::unordered_map<AnimationMediatorStateEntry*, AnimationMediatorStateEntry*> AnimationMediatorStateEntry_already_saved_links1;
static std::unordered_map<AnimationMediatorStateEntry*, AnimationMediatorStateEntry*> AnimationMediatorStateEntry_already_saved_links2;

void copy_AnimationMediator(AnimationMediator* to, AnimationMediator* from)
{
    for (int i = 0; i < 31; i++)
    {
        AnimationMediatorStateEntry_already_saved_links1.clear();
        AnimationMediatorStateEntry_already_saved_links2.clear();
        copy_AnimationMediatorStateEntry(&to->states_list[i], &from->states_list[i], AnimationMediatorStateEntry_already_saved_links1, AnimationMediatorStateEntry_already_saved_links2);
    }
    copy_AnimationQueue(to->animationQueue, from->animationQueue);
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
}

AnimationMediator* init_AnimationMediator()
{
    AnimationMediator* local_AnimationMediator = (AnimationMediator*)malloc_(sizeof(AnimationMediator));

    for (int i = 0; i < 31; i++)
    {
        AnimationMediatorStateEntry* pAnimationMediatorStateEntry = init_AnimationMediatorStateEntry(0);
        local_AnimationMediator->states_list[i] = *pAnimationMediatorStateEntry;
        free(pAnimationMediatorStateEntry);
    }
    local_AnimationMediator->animationQueue = init_AnimationQueue();

    return local_AnimationMediator;
}

void copy_AnimationMediatorStateEntry(AnimationMediatorStateEntry* to, AnimationMediatorStateEntry* from, std::unordered_map<AnimationMediatorStateEntry*, AnimationMediatorStateEntry*>& links1, std::unordered_map<AnimationMediatorStateEntry*, AnimationMediatorStateEntry*>& links2)
{
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
    memcpy(to->data_1, from->data_1, sizeof(to->data_1));
    if (from->linked_animation1 != NULL)
    {
        //if we have already saved this linked animation, close the pointer loop. linked animations can have cycles
        auto saved_link1 = links1.find(from->linked_animation1);
        if (saved_link1 != links1.end())
        {
            to->linked_animation1 = saved_link1->second;
        }
        else
        {
            if (to->linked_animation1 == NULL)
            {
                ConsoleWrite("Unable to recursivly copy AnimationMediatorStateEntry into to->linked_animation1. Pointer is null");
            }
            else
            {
                links1.emplace(from->linked_animation1, to->linked_animation1);
                copy_AnimationMediatorStateEntry(to->linked_animation1, from->linked_animation1, links1, links2);
            }
        }
    }
    if (from->linked_animation2 != NULL)
    {
        auto saved_link2 = links2.find(from->linked_animation2);
        if (saved_link2 != links2.end())
        {
            to->linked_animation2 = saved_link2->second;
        }
        else
        {
            if (to->linked_animation2 == NULL)
            {
                ConsoleWrite("Unable to recursivly copy AnimationMediatorStateEntry into to->linked_animation2. Pointer is null");
            }
            else
            {
                links2.emplace(from->linked_animation2, to->linked_animation2);
                copy_AnimationMediatorStateEntry(to->linked_animation2, from->linked_animation2, links1, links2);
            }
        }
    }
}

AnimationMediatorStateEntry* init_AnimationMediatorStateEntry(size_t depth)
{
    //this is a tree of linked animations. allow for a max tree depth of X means we must have 2^x - 1 linked animation state entries.
    //only allow a max depth of 5, or 31 nodes
    if (depth == 5)
    {
        return NULL;
    }
    AnimationMediatorStateEntry* local_AnimationMediatorStateEntry = (AnimationMediatorStateEntry*)malloc_(sizeof(AnimationMediatorStateEntry));

    local_AnimationMediatorStateEntry->linked_animation1 = init_AnimationMediatorStateEntry(depth + 1);
    local_AnimationMediatorStateEntry->linked_animation2 = init_AnimationMediatorStateEntry(depth + 1);

    return local_AnimationMediatorStateEntry;
}

void copy_ChrCtrl_AnimationQueue(ChrCtrl_AnimationQueue* to, ChrCtrl_AnimationQueue* from)
{
    //we allow up to a max of 32 ChrCtrl_AnimationQueue_field0x8 entries in the array
    if (from->array_length > 32)
    {
        FATALERROR("Got %d number of ChrCtrl_AnimationQueue_field0x8 entries for ChrCtrl_AnimationQueue->arry. Only support a max of 32.");
    }
    to->array_length = from->array_length;
    to->data_0 = from->data_0;
    for (size_t i = 0; i < from->array_length; i++)
    {
        copy_ChrCtrl_AnimationQueue_field0x8(&to->arry[i], &from->arry[i]);
    }
    memcpy(to->data_1, from->data_1, sizeof(to->data_1));
    memcpy(to->data_2, from->data_2, sizeof(to->data_2));
    to->data_3 = from->data_3;
    memcpy(to->data_4, from->data_4, sizeof(to->data_4));
    memcpy(to->data_5, from->data_5, sizeof(to->data_5));
}

ChrCtrl_AnimationQueue* init_ChrCtrl_AnimationQueue()
{
    ChrCtrl_AnimationQueue* local_ChrCtrl_AnimationQueue = (ChrCtrl_AnimationQueue*)malloc_(sizeof(ChrCtrl_AnimationQueue));
    ChrCtrl_AnimationQueue_field0x8* local_ChrCtrl_AnimationQueue_field0x8 = (ChrCtrl_AnimationQueue_field0x8*)malloc_(sizeof(ChrCtrl_AnimationQueue_field0x8) * 32);

    local_ChrCtrl_AnimationQueue->arry = local_ChrCtrl_AnimationQueue_field0x8;

    return local_ChrCtrl_AnimationQueue;
}

void copy_ChrCtrl_AnimationQueue_field0x8(ChrCtrl_AnimationQueue_field0x8* to, ChrCtrl_AnimationQueue_field0x8* from)
{
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
    memcpy(to->data_1, from->data_1, sizeof(to->data_1));
}

void copy_AnimationQueue(AnimationQueue* to, AnimationQueue* from)
{
    for (int i = 0; i < 6; i++)
    {
        copy_AnimationQueue_Entry(to->AnimationQueue_Entries[i], from->AnimationQueue_Entries[i]);
    }
}

AnimationQueue* init_AnimationQueue()
{
    AnimationQueue* local_AnimationQueue = (AnimationQueue*)malloc_(sizeof(AnimationQueue));

    for (int i = 0; i < 6; i++)
    {
        local_AnimationQueue->AnimationQueue_Entries[i] = init_AnimationQueue_Entry();
    }

    return local_AnimationQueue;
}

void copy_AnimationQueue_Entry(AnimationQueue_Entry* to, AnimationQueue_Entry* from)
{
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
    memcpy(to->data_1, from->data_1, sizeof(to->data_1));
    memcpy(to->data_2, from->data_2, sizeof(to->data_2));
    memcpy(to->data_3, from->data_3, sizeof(to->data_3));
}

AnimationQueue_Entry* init_AnimationQueue_Entry()
{
    AnimationQueue_Entry* local_AnimationQueue_Entry = (AnimationQueue_Entry*)malloc_(sizeof(AnimationQueue_Entry));
    return local_AnimationQueue_Entry;
}
