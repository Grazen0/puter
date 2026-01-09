#include "control.h"
#include "io.h"
#include "keyboard.h"
#include "numeric.h"
#include "puter.h"
#include "riscv.h"
#include "rtc.h"
#include "vga.h"
#include <stddef.h>

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

    printk(
        "\033[0;30mBlack   \033[0;90mBright Black   \033[0;40mBlack   \033[0;100mBright Black   \n"
        "\033[0;31mRed     \033[0;91mBright Red     \033[0;41mRed     \033[0;101mBright Red     \n"
        "\033[0;32mGreen   \033[0;92mBright Green   \033[0;42mGreen   \033[0;102mBright Green   \n"
        "\033[0;33mYellow  \033[0;93mBright Yellow  \033[0;43mYellow  \033[0;103mBright Yellow  \n"
        "\033[0;34mBlue    \033[0;94mBright Blue    \033[0;44mBlue    \033[0;104mBright Blue    \n"
        "\033[0;35mPurple  \033[0;95mBright Purple  \033[0;45mPurple  \033[0;105mBright Purple  \n"
        "\033[0;36mCyan    \033[0;96mBright Cyan    \033[0;46mCyan    \033[0;106mBright Cyan    \n"
        "\033[0;37mWhite   \033[0;97mBright White   \033[0;47mWhite   \033[0;107mBright White   \n"
        "\033[0;m");

    Key key = {};

    while (true) {
        kb_process_queue();

        while (kb_poll_key(&key))
            printk("key: %d, mod: %08X\n", key.code, key.mod);
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
