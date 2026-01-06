#include "control.h"
#include "keyboard.h"
#include "numeric.h"
#include "puter.h"
#include "riscv.h"
#include "rtc.h"
#include "sd_card.h"
#include "vga.h"
#include <stddef.h>
#include <stdio.h>

static const char banner[] = "\
 ____        _             ___  ____   \n\
|  _ \\ _   _| |_ ___ _ __ / _ \\/ ___|  \n\
| |_) | | | | __/ _ \\ '__| | | \\___ \\  \n\
|  __/| |_| | ||  __/ |  | |_| |___) | \n\
|_|    \\__,_|\\__\\___|_|   \\___/|____/  \n\
";

void kmain()
{
    vga_print("kernel!\n");
    __asm__ volatile("csrr t0, mepc"); // should trigger illegal instruction exception
    PANIC("should not have reached here");
}

void main()
{
    vga_init();

    vga_print("Initializing RTC...\n");
    rtc_init();

    vga_print("Initializing PLIC...\n");
    for (size_t i = 0; i < PLIC_PORTS; ++i) {
        PLIC->int_enable[i] = 1;
        PLIC->int_priority[i] = 1 + i;
    }

    vga_print("Initializing keyboard driver...\n");
    kb_init();

    vga_print("Initializing SD card...\n");
    const SdInitResult sd_result = sd_init();

    if (sd_result != SdInitResult_Ok) {
        vga_print("Could not initialize SD card: ");
        vga_print(sd_init_result_str(sd_result));
        vga_print("\n");
    }

    vga_print("Enabling interrupts...\n");
    rv_set_mie(MieBit_Timer | MieBit_External);
    rv_set_mstatus(MStatus_Mie);

    vga_print("\n");

    vga_print("Wake up, Neo...\n");
    vga_print("\n");
    vga_print(banner);
    vga_print("\n");
    vga_print("Welcome to PuterOS.\n");
    vga_print("\n");

    for (u8 i = 0; i < 16; ++i)
        TRAM[i].attr = i << 4;

    static u8 block_buf[SD_BLOCK_SIZE];
    sd_read_block(0, block_buf);

    Key key = {};

    while (true) {
        kb_process_queue();

        while (kb_poll_key(&key)) {
            printf("key: %d, mod: %08X\n", key.code, key.mod);
        }
    }
}

[[gnu::interrupt]] void trap_handler()
{
    const u32 mcause = rv_read_mcause();

    switch (mcause) {
    case MCause_MTimerInt:
        rtc_process_interrupt();
        break;

    case MCause_MExternalInt:
        const u8 int_id = MEIID;

        PLIC->int_claim[int_id] = 1;

        if (int_id == MeiId_Keyboard)
            kb_process_interrupt();

        break;

    case MCause_IllegalInstr:
        PANIC("Illegal instruction (pc = 0x%08X)\n", rv_read_mepc());

    case MCause_UEcall:
        vga_print("User ecall\n");
        rv_inc_mepc();
        break;

    default:
        PANIC("Unknown trap (mcause = 0x%08X)\n", mcause);
    }
}
