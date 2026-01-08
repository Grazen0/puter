#include "control.h"
#include "io.h"
#include "keyboard.h"
#include "puter.h"
#include "riscv.h"
#include "rtc.h"
#include "vga.h"

static constexpr char BANNER[] = "\
 ____        _             ___  ____   \n\
|  _ \\ _   _| |_ ___ _ __ / _ \\/ ___|  \n\
| |_) | | | | __/ _ \\ '__| | | \\___ \\  \n\
|  __/| |_| | ||  __/ |  | |_| |___) | \n\
|_|    \\__,_|\\__\\___|_|   \\___/|____/  \n\
";

void main()
{
    vga_init();

    printk("Initializing RTC...\n");
    rtc_init();

    printk("Initializing PLIC...\n");
    for (size_t i = 0; i < PLIC_PORTS; ++i) {
        PLIC->int_enable[i] = 1;
        PLIC->int_priority[i] = 1 + i;
    }

    printk("Initializing keyboard driver...\n");
    kb_init();

    printk("Enabling interrupts...\n");
    rv_set_mie(MieBit_Timer | MieBit_External);
    rv_set_mstatus(MStatus_Mie);

    printk("\n");

    printk("Wake up, Neo...\n");
    printk("\n");
    printk("%s\n", BANNER);
    printk("Welcome to PuterOS.\n");
    printk("\n");

    for (u8 i = 0; i < 16; ++i)
        TRAM[i].attr = i << 4;

    Key key = {};

    while (true) {
        kb_process_queue();

        while (kb_poll_key(&key)) {
            printk("key: %d, mod: %08X\n", key.code, key.mod);
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
        printk("User ecall\n");
        rv_inc_mepc();
        break;

    default:
        PANIC("Unknown trap (mcause = 0x%08X)\n", mcause);
    }
}
