#include "Rollback.h"
#include "SP/memory.h"
#include "SP/memory/injection/asm/x64.h"
#include "DarkSoulsOverhaulMod.h"
#include "MainLoop.h"
#include "InputUtil.h"

extern "C" {
    uint64_t sendType1NetMessage_injection_return;
    void sendType1NetMessage_injection();
    uint64_t recvType1NetMessage_injection_return;
    void recvType1NetMessage_injection();
}

PlayerIns* Rollback::saved_playerins = NULL;
PadMan* Rollback::saved_padman = NULL;

bool Rollback::bsave = false;
bool Rollback::bload = false;
bool rollback_test(void* unused)
{
    if (Rollback::bsave)
    {
        Rollback::GameStateSave();
        Rollback::bsave = false;
    }

    if (Rollback::bload)
    {
        Rollback::GameStateLoad();
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

    //TMP init out copy of the playerins struct, for saving/restoring with rollback
    Rollback::saved_playerins = init_PlayerIns();
    Rollback::saved_padman = init_PadMan();

    //TMP save/restore with a hotkey
    MainLoop::setup_mainloop_callback(rollback_test, NULL, "rollback_test");
}
