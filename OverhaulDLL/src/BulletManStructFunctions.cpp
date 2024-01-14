#include "BulletManStructFunctions.h"
#include "SfxManStructFunctions.h"
#include "Rollback.h"

typedef void* falloc(uint64_t, uint64_t, uint32_t);

std::string print_BulletMan(BulletMan* to)
{
    std::string out = "BulletMan\n";
    if (to == NULL)
    {
        return out;
    }

    for (size_t i = 0; i < 128; i++)
    {
        out += print_BulletIns(&to->bulletins_arry[i]);
    }

    out += "Data 0:";
    for (size_t i = 0; i < sizeof(to->data_0)/sizeof(to->data_0[0]); i++)
    {
        out += std::to_string(to->data_0[i]);
        out += " ";
    }
    out += "\n";

    out += print_BulletMan_Field0x20(to->field0x20);

    out += "Data 1:";
    for (size_t i = 0; i < sizeof(to->data_1) / sizeof(to->data_1[0]); i++)
    {
        out += std::to_string(to->data_1[i]);
        out += " ";
    }
    out += "\n";

    out += print_BulletMan_Field0x40(to->field0x40);

    out += "Data 2:";
    for (size_t i = 0; i < sizeof(to->data_2) / sizeof(to->data_2[0]); i++)
    {
        out += std::to_string(to->data_2[i]);
        out += " ";
    }
    out += "\n";

    out += print_ChrCam(to->chrCam);

    out += "Data 3:" + std::to_string(to->data_3) + "\n";

    if (to->field0x78 != NULL && to->field0x78_end != NULL)
    {
        size_t field0x78_len = (to->field0x78_end - (uint64_t)to->field0x78) / 8;
        for (size_t i = 0; i < field0x78_len; i++)
        {
            out += print_BulletMan_field0x78Elem(to->field0x78[i]);
        }
    }

    out += "Data 5:";
    for (size_t i = 0; i < sizeof(to->data_5) / sizeof(to->data_5[0]); i++)
    {
        out += std::to_string(to->data_5[i]);
        out += " ";
    }
    out += "\n";

    return out;
}

void copy_BulletMan(BulletMan* to, BulletMan* from, bool to_game)
{
    Game::SuspendThreads();

    for (size_t i = 0; i < 128; i++)
    {
        copy_BulletIns(&to->bulletins_arry[i], &from->bulletins_arry[i], to_game);
    }

    memcpy(to->data_0, from->data_0, sizeof(to->data_0));

    for (size_t i = 0; i < 40; i++)
    {
        copy_BulletMan_Field0x20(&to->field0x20[i], &from->field0x20[i], to_game);
    }

    memcpy(to->data_1, from->data_1, sizeof(to->data_1));

    for (size_t i = 0; i < 4; i++)
    {
        copy_BulletMan_Field0x40(&to->field0x40[i], &from->field0x40[i], to_game);
    }

    memcpy(to->data_2, from->data_2, sizeof(to->data_2));

    copy_ChrCam(to->chrCam, from->chrCam, to_game);

    to->data_3 = from->data_3;

    if (from->field0x78 != NULL && from->field0x78_end != NULL)
    {
        size_t field0x78_len = (from->field0x78_end - (uint64_t)from->field0x78) / 8;
        if (field0x78_len > 3)
        {
            FATALERROR("BulletMan->field0x78 array is longer then 3 elements. end=%x start=%x len=%d", from->field0x78_end, from->field0x78, field0x78_len);
        }
        for (size_t i = 0; i < field0x78_len; i++)
        {
            copy_BulletMan_field0x78Elem(to->field0x78[i], from->field0x78[i], to_game);
        }
    }
    else
    {
        if (to_game)
        {
            to->field0x78 = NULL;
        }
    }
    to->field0x78_next = (uint64_t)(to->field0x78) + (from->field0x78_next - (uint64_t)from->field0x78); //convert to offset and then to pointer
    to->field0x78_end = (uint64_t)(to->field0x78) + (from->field0x78_end - (uint64_t)from->field0x78);

    memcpy(to->data_5, from->data_5, sizeof(to->data_5));

    Game::ResumeThreads();
}

BulletMan* init_BulletMan()
{
    BulletMan* local_BulletMan = (BulletMan*)malloc_(sizeof(BulletMan));

    local_BulletMan->bulletins_arry = (BulletIns*)malloc_(sizeof(BulletIns) * 128);
    for (size_t i = 0; i < 128; i++)
    {
        BulletIns* local_bulletins = init_BulletIns();
        local_BulletMan->bulletins_arry[i] = *local_bulletins;
        free(local_bulletins);
    }

    local_BulletMan->field0x20 = (BulletMan_Field0x20*)malloc_(sizeof(BulletMan_Field0x20) * 40);
    for (size_t i = 0; i < 40; i++)
    {
        BulletMan_Field0x20* local_BulletMan_Field0x20 = init_BulletMan_Field0x20();
        local_BulletMan->field0x20[i] = *local_BulletMan_Field0x20;
        free(local_BulletMan_Field0x20);
    }

    local_BulletMan->field0x40 = (BulletMan_Field0x40*)malloc_(sizeof(BulletMan_Field0x40) * 4);
    for (size_t i = 0; i < 4; i++)
    {
        BulletMan_Field0x40* local_BulletMan_Field0x40 = init_BulletMan_Field0x40();
        local_BulletMan->field0x40[i] = *local_BulletMan_Field0x40;
        free(local_BulletMan_Field0x40);
    }

    local_BulletMan->chrCam = init_ChrCam();

    local_BulletMan->field0x78 = (BulletMan_field0x78Elem**)malloc_(sizeof(BulletMan_field0x78Elem*) * 3);
    for (size_t i = 0; i < 3; i++)
    {
        local_BulletMan->field0x78[i] = init_BulletMan_field0x78Elem();
    }

    return local_BulletMan;
}

void free_BulletMan(BulletMan* to)
{
    for (size_t i = 0; i < 128; i++)
    {
        free_BulletIns(&to->bulletins_arry[i], false);
    }
    free(to->bulletins_arry);
    for (size_t i = 0; i < 40; i++)
    {
        free_BulletMan_Field0x20(&to->field0x20[i], false);
    }
    free(to->field0x20);
    for (size_t i = 0; i < 4; i++)
    {
        free_BulletMan_Field0x40(&to->field0x40[i], false);
    }
    free(to->field0x40);
    for (size_t i = 0; i < 3; i++)
    {
        free_BulletMan_field0x78Elem(to->field0x78[i]);
    }
    free_ChrCam(to->chrCam);
    free(to->field0x78);

    free(to);
}

std::string print_ChrCam(ChrCam* to)
{
    std::string out = "ChrCam\n";
    if (to == NULL)
    {
        return out;
    }

    out += print_ChrExFollowCam(to->chrExFollowCam);

    return out;
}

void copy_ChrCam(ChrCam* to, ChrCam* from, bool to_game)
{
    copy_ChrExFollowCam(to->chrExFollowCam, from->chrExFollowCam, to_game);
}

ChrCam* init_ChrCam()
{
    ChrCam* local_ChrCam = (ChrCam*)malloc_(sizeof(ChrCam));
    local_ChrCam->chrExFollowCam = init_ChrExFollowCam();
    return local_ChrCam;
}

void free_ChrCam(ChrCam* to)
{
    free_ChrExFollowCam(to->chrExFollowCam);
    free(to);
}

std::string print_ChrExFollowCam(ChrExFollowCam* to)
{
    std::string out = "ChrExFollowCam\n";
    if (to == NULL)
    {
        return out;
    }

    out += "Data 0:";
    for (size_t i = 0; i < sizeof(to->data_0); i++)
    {
        out += std::to_string(to->data_0[i]);
        out += " ";
    }
    out += "\n";

    return out;
}

void copy_ChrExFollowCam(ChrExFollowCam* to, ChrExFollowCam* from, bool to_game)
{
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
}

ChrExFollowCam* init_ChrExFollowCam()
{
    ChrExFollowCam* local_ChrExFollowCam = (ChrExFollowCam*)malloc_(sizeof(ChrExFollowCam));
    return local_ChrExFollowCam;
}

void free_ChrExFollowCam(ChrExFollowCam* to)
{
    free(to);
}

std::string print_BulletIns(BulletIns* to)
{
    std::string out = "BulletIns\n";
    if (to == NULL)
    {
        return out;
    }

    out += "Data 0:";
    for (size_t i = 0; i < sizeof(to->data_0); i++)
    {
        out += std::to_string(to->data_0[i]);
        out += " ";
    }
    out += "\n";

    out += print_BulletIns_FollowupBullet(&to->FollowupBullet);

    out += "Data 1:" + std::to_string(to->data_1) + "\n";

    out += print_BulletIns_Field0x90_Field0x1a0(&to->owner);

    out += "Data 2:";
    for (size_t i = 0; i < sizeof(to->data_2); i++)
    {
        out += std::to_string(to->data_2[i]);
        out += " ";
    }
    out += "\n";

    out += print_BulletTargetingSystemOwner(&to->bulletTargetingSystemOwner);

    out += print_TargetingSystemBase(&to->targetingSystemBase);

    out += "Data 3:";
    for (size_t i = 0; i < sizeof(to->data_3); i++)
    {
        out += std::to_string(to->data_3[i]);
        out += " ";
    }
    out += "\n";

    out += print_BulletState(&to->bulletState);

    out += "Data 4:" + std::to_string(to->data_4) + "\n";

    out += print_BulletFlyState(&to->bulletFlyState);

    out += print_BulletState(&to->bulletExplosionState);

    out += "Data 5:" + std::to_string(to->data_5) + "\n";

    return out;
}

void copy_BulletIns(BulletIns* to, BulletIns* from, bool to_game)
{
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
    copy_BulletIns_FollowupBullet(&to->FollowupBullet, &from->FollowupBullet, to_game);
    to->data_1 = from->data_1;
    copy_BulletIns_Field0x90_Field0x1a0(&to->owner, &from->owner, to_game);
    memcpy(to->data_2, from->data_2, sizeof(to->data_2));
    copy_BulletTargetingSystemOwner(&to->bulletTargetingSystemOwner, &from->bulletTargetingSystemOwner, to_game);
    copy_TargetingSystemBase(&to->targetingSystemBase, &from->targetingSystemBase, to_game);
    memcpy(to->data_3, from->data_3, sizeof(to->data_3));
    copy_BulletState(&to->bulletState, &from->bulletState, to_game);
    to->data_4 = from->data_4;
    copy_BulletFlyState(&to->bulletFlyState, &from->bulletFlyState, to_game);
    copy_BulletState(&to->bulletExplosionState, &from->bulletExplosionState, to_game);
    to->padding_previous_bullet_in_use = NULL;
    to->data_5 = from->data_5;
}

BulletIns* init_BulletIns()
{
    BulletIns* local_BulletIns = (BulletIns*)malloc_(sizeof(BulletIns));

    return local_BulletIns;
}

void free_BulletIns(BulletIns* to, bool freeself)
{
    free_BulletIns_FollowupBullet(&to->FollowupBullet, false, true);
    if (freeself)
    {
        free(to);
    }
}

static uint64_t* HeapPtr = (uint64_t*)(0x0141B67450 + 8);

std::string print_BulletIns_FollowupBullet(BulletIns_FollowupBullet* to)
{
    std::string out = "BulletIns_FollowupBullet\n";
    if (to == NULL)
    {
        return out;
    }

    out += "FXManager:" + std::to_string(to->FXManager) + "\n";

    out += print_FXEntry_Substruct(to->FXEntry_Substruct_a);

    out += print_FXEntry_Substruct(to->FXEntry_Substruct_b);

    if (to->next != NULL)
    {
        out += print_BulletIns_FollowupBullet(to->next);
    }

    return out;
}

void copy_BulletIns_FollowupBullet(BulletIns_FollowupBullet* to, BulletIns_FollowupBullet* from, bool to_game)
{
    //Assuming that we're already saving/restoring SfxMan, then this is just a const ptr to the FXManager in the SfxMan global
    to->FXManager = from->FXManager;

    //FXEntry_Substruct_a
    if (from->FXEntry_Substruct_a != NULL)
    {
        if (to->FXEntry_Substruct_a == NULL)
        {
            if (to_game)
            {
                to->FXEntry_Substruct_a = (FXEntry_Substruct*)smallObject_internal_malloc(*HeapPtr, sizeof(FXEntry_Substruct), 8);
            }
            else
            {
                to->FXEntry_Substruct_a = init_FXEntry_Substruct();
            }
        }
        //should just be able to treat the parent as a raw ptr i think
        //assuming we're also saveing/restoring the sfxman, it should ptr to an object in that
        copy_FXEntry_Substruct(to->FXEntry_Substruct_a, from->FXEntry_Substruct_a, to_game, from->FXEntry_Substruct_a->parent);
    }
    if (from->FXEntry_Substruct_a == NULL)
    {
        if (to->FXEntry_Substruct_a != NULL)
        {
            if (to_game)
            {
                smallObject_internal_dealloc(*HeapPtr, to->FXEntry_Substruct_a, sizeof(FXEntry_Substruct), 8);
            }
            else
            {
                free_FXEntry_Substruct(to->FXEntry_Substruct_a);
            }
            to->FXEntry_Substruct_a = NULL;
        }
    }

    //FXEntry_Substruct_b
    if (from->FXEntry_Substruct_b != NULL)
    {
        if (to->FXEntry_Substruct_b == NULL)
        {
            if (to_game)
            {
                to->FXEntry_Substruct_b = (FXEntry_Substruct*)smallObject_internal_malloc(*HeapPtr, sizeof(FXEntry_Substruct), 8);
            }
            else
            {
                to->FXEntry_Substruct_b = init_FXEntry_Substruct();
            }
        }
        copy_FXEntry_Substruct(to->FXEntry_Substruct_b, from->FXEntry_Substruct_b, to_game, from->FXEntry_Substruct_b->parent);
    }
    if (from->FXEntry_Substruct_b == NULL)
    {
        if (to->FXEntry_Substruct_b != NULL)
        {
            if (to_game)
            {
                smallObject_internal_dealloc(*HeapPtr, to->FXEntry_Substruct_b, sizeof(FXEntry_Substruct), 8);
            }
            else
            {
                free_FXEntry_Substruct(to->FXEntry_Substruct_b);
            }
            to->FXEntry_Substruct_b = NULL;
        }
    }

    //the next/prev ptrs have to be handled by the caller

    return;
}

void free_BulletIns_FollowupBullet(BulletIns_FollowupBullet* to, bool freeself, bool freenext)
{
    if (to->FXEntry_Substruct_a != NULL)
    {
        free_FXEntry_Substruct(to->FXEntry_Substruct_a);
    }

    if (freenext && to->next != NULL)
    {
        free_BulletIns_FollowupBullet(to->next, true, freenext);
    }

    if (to->FXEntry_Substruct_b != NULL)
    {
        free_FXEntry_Substruct(to->FXEntry_Substruct_b);
    }

    if (freeself)
    {
        free(to);
    }
}

std::string print_BulletIns_Field0x90_Field0x1a0(BulletIns_Field0x90_Field0x1a0* to)
{
    std::string out = "BulletIns_Field0x90_Field0x1a0\n";
    if (to == NULL)
    {
        return out;
    }

    out += "Data 0:";
    for (size_t i = 0; i < sizeof(to->data_0); i++)
    {
        out += std::to_string(to->data_0[i]);
        out += " ";
    }
    out += "\n";

    out += "Data 1:";
    for (size_t i = 0; i < sizeof(to->data_1); i++)
    {
        out += std::to_string(to->data_1[i]);
        out += " ";
    }
    out += "\n";

    out += "Data 2:";
    for (size_t i = 0; i < sizeof(to->data_2); i++)
    {
        out += std::to_string(to->data_2[i]);
        out += " ";
    }
    out += "\n";

    out += "Data 3:";
    for (size_t i = 0; i < sizeof(to->data_3); i++)
    {
        out += std::to_string(to->data_3[i]);
        out += " ";
    }
    out += "\n";

    out += "Data 4:";
    for (size_t i = 0; i < sizeof(to->data_4); i++)
    {
        out += std::to_string(to->data_4[i]);
        out += " ";
    }
    out += "\n";

    out += "Data 5:";
    for (size_t i = 0; i < sizeof(to->data_5); i++)
    {
        out += std::to_string(to->data_5[i]);
        out += " ";
    }
    out += "\n";

    return out;
}

void copy_BulletIns_Field0x90_Field0x1a0(BulletIns_Field0x90_Field0x1a0* to, BulletIns_Field0x90_Field0x1a0* from, bool to_game)
{
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
    memcpy(to->data_1, from->data_1, sizeof(to->data_1));
    memcpy(to->data_2, from->data_2, sizeof(to->data_2));
    memcpy(to->data_3, from->data_3, sizeof(to->data_3));
    memcpy(to->data_4, from->data_4, sizeof(to->data_4));
    memcpy(to->data_5, from->data_5, sizeof(to->data_5));
}

std::string print_BulletTargetingSystemOwner(BulletTargetingSystemOwner* to)
{
    std::string out = "BulletTargetingSystemOwner\n";
    if (to == NULL)
    {
        return out;
    }

    out += "Data 0:";
    for (size_t i = 0; i < sizeof(to->data_0); i++)
    {
        out += std::to_string(to->data_0[i]);
        out += " ";
    }
    out += "\n";

    return out;
}

void copy_BulletTargetingSystemOwner(BulletTargetingSystemOwner* to, BulletTargetingSystemOwner* from, bool to_game)
{
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
}

std::string print_TargetingSystemBase(TargetingSystemBase* to)
{
    std::string out = "TargetingSystemBase\n";
    if (to == NULL)
    {
        return out;
    }

    out += "Data 0:";
    for (size_t i = 0; i < sizeof(to->data_0); i++)
    {
        out += std::to_string(to->data_0[i]);
        out += " ";
    }
    out += "\n";

    out += "Data 1:" + std::to_string(to->data_1) + "\n";

    return out;
}

void copy_TargetingSystemBase(TargetingSystemBase* to, TargetingSystemBase* from, bool to_game)
{
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
    to->data_1 = from->data_1;
}

std::string print_BulletState(BulletState* to)
{
    std::string out = "BulletState\n";
    if (to == NULL)
    {
        return out;
    }

    out += print_BulletParamInfo(&to->paramInfo);

    out += "Data 0:" + std::to_string(to->data_0) + "\n";

    return out;
}

void copy_BulletState(BulletState* to, BulletState* from, bool to_game)
{
    copy_BulletParamInfo(&to->paramInfo, &from->paramInfo, to_game);
    to->data_0 = from->data_0;
}

std::string print_BulletFlyState(BulletFlyState* to)
{
    std::string out = "BulletFlyState\n";
    if (to == NULL)
    {
        return out;
    }

    out += print_BulletState(&to->state);

    out += "Data 0:" + std::to_string(to->data_0) + "\n";

    return out;
}

void copy_BulletFlyState(BulletFlyState* to, BulletFlyState* from, bool to_game)
{
    copy_BulletState(&to->state, &from->state, to_game);
    to->data_0 = from->data_0;
}

std::string print_BulletParamInfo(BulletParamInfo* to)
{
    std::string out = "BulletParamInfo\n";
    if (to == NULL)
    {
        return out;
    }

    out += "Data 0:";
    for (size_t i = 0; i < sizeof(to->data_0); i++)
    {
        out += std::to_string(to->data_0[i]);
        out += " ";
    }
    out += "\n";

    return out;
}

void copy_BulletParamInfo(BulletParamInfo* to, BulletParamInfo* from, bool to_game)
{
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
}

std::string print_BulletMan_Field0x20(BulletMan_Field0x20* to)
{
    std::string out = "BulletParamInfo\n";
    if (to == NULL)
    {
        return out;
    }

    out += "Data 0:";
    for (size_t i = 0; i < sizeof(to->data_0); i++)
    {
        out += std::to_string(to->data_0[i]);
        out += " ";
    }
    out += "\n";

    out += print_BulletParamInfo(to->bulletParamInfo);

    out += print_BulletIns_Field0x90_Field0x1a0(&to->field0x1a0);

    out += "Data 1:";
    for (size_t i = 0; i < sizeof(to->data_1)/sizeof(to->data_1[0]); i++)
    {
        out += std::to_string(to->data_1[i]);
        out += " ";
    }
    out += "\n";

    return out;
}

void copy_BulletMan_Field0x20(BulletMan_Field0x20* to, BulletMan_Field0x20* from, bool to_game)
{
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
    copy_BulletParamInfo(to->bulletParamInfo, from->bulletParamInfo, to_game);
    copy_BulletIns_Field0x90_Field0x1a0(&to->field0x1a0, &from->field0x1a0, to_game);
    memcpy(to->data_1, from->data_1, sizeof(to->data_1));
}

BulletMan_Field0x20* init_BulletMan_Field0x20()
{
    BulletMan_Field0x20* local_BulletMan_Field0x20 = (BulletMan_Field0x20*)malloc_(sizeof(BulletMan_Field0x20));

    local_BulletMan_Field0x20->bulletParamInfo = (BulletParamInfo*)malloc_(sizeof(BulletParamInfo));

    return local_BulletMan_Field0x20;

}

void free_BulletMan_Field0x20(BulletMan_Field0x20* to, bool freeself)
{
    free(to->bulletParamInfo);

    if (freeself)
    {
        free(to);
    }
}

std::string print_BulletMan_Field0x40(BulletMan_Field0x40* to)
{
    std::string out = "BulletParamInfo\n";
    if (to == NULL)
    {
        return out;
    }

    out += "Data 0:" + std::to_string(to->data_0) + "\n";

    out += "Data 1:";
    for (size_t i = 0; i < sizeof(to->data_1)/sizeof(to->data_1[0]); i++)
    {
        out += std::to_string(to->data_1[i]);
        out += " ";
    }
    out += "\n";

    return out;
}

void copy_BulletMan_Field0x40(BulletMan_Field0x40* to, BulletMan_Field0x40* from, bool to_game)
{
    to->data_0 = from->data_0;
    memcpy(to->data_1, from->data_1, sizeof(to->data_1));
}

BulletMan_Field0x40* init_BulletMan_Field0x40()
{
    BulletMan_Field0x40* local_BulletMan_Field0x40 = (BulletMan_Field0x40*)malloc_(sizeof(BulletMan_Field0x40));

    return local_BulletMan_Field0x40;
}

void free_BulletMan_Field0x40(BulletMan_Field0x40* to, bool freeself)
{
    if (freeself)
    {
        free(to);
    }
}

std::string print_BulletMan_field0x78Elem(BulletMan_field0x78Elem* to)
{
    std::string out = "BulletParamInfo\n";
    if (to == NULL)
    {
        return out;
    }

    out += "Data 0:";
    for (size_t i = 0; i < sizeof(to->data_0); i++)
    {
        out += std::to_string(to->data_0[i]);
        out += " ";
    }
    out += "\n";

    return out;
}

void copy_BulletMan_field0x78Elem(BulletMan_field0x78Elem* to, BulletMan_field0x78Elem* from, bool to_game)
{
    memcpy(to->data_0, from->data_0, sizeof(to->data_0));
}

BulletMan_field0x78Elem* init_BulletMan_field0x78Elem()
{
    BulletMan_field0x78Elem* local_BulletMan_field0x78Elem = (BulletMan_field0x78Elem*)malloc_(sizeof(BulletMan_field0x78Elem));

    return local_BulletMan_field0x78Elem;
}

void free_BulletMan_field0x78Elem(BulletMan_field0x78Elem* to)
{
    free(to);
}
