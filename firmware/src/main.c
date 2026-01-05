#include "keyboard.h"
#include "numeric.h"
#include "puter.h"
#include "riscv.h"
#include "rtc.h"
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
    printf("kernel!\n");
    __asm__ volatile("csrr t0, mepc");
    while (true) {
    }
}

void main()
{
    vga_init();

    printf("Initializing RTC...\n");
    rtc_init();

    printf("Initializing PLIC...\n");
    for (size_t i = 0; i < PLIC_PORTS; ++i) {
        PLIC->int_enable[i] = 1;
        PLIC->int_priority[i] = 1 + i;
    }

    printf("Initializing keyboard driver...\n");
    kb_init();

    printf("Enabling interrupts...\n");
    rv_mie_set(MIE_TIMER | MIE_EXTERNAL);
    rv_mstatus_set(MSTATUS_MIE);

    printf("\n");

    printf("Wake up, Neo...\n");
    printf("\n");
    printf("%s", banner);
    printf("\n");
    printf("Welcome to PuterOS.\n");
    printf("\n");

    for (u8 i = 0; i < 16; ++i)
        TRAM[i].attr = i << 4;

    while (true) {
        kb_process_queue();

        Key key;

        while (kb_poll_key(&key)) {
            printf("key: %i, mod: %08X\n", key.code, key.mod);
        }
    }
}

[[gnu::interrupt]] void trap_handler()
{
    // TODO: set up reentrancy

    const u32 mcause = rv_mcause_read();

    switch (mcause) {
    case MCAUSE_M_TIMER_INT:
        rtc_process_interrupt();
        break;

    case MCAUSE_M_EXTERNAL_INT:
        const u8 int_id = MEIID;
        PLIC->int_claim[int_id] = 1;

        if (int_id == MEIID_KEYBOARD)
            kb_process_interrupt();

        break;

    case MCAUSE_ILLEGAL_INSTR:
        printf("Illegal instruction (pc = 0x%08X)\n", rv_mepc_read());
        while (true) {
        }

    case MCAUSE_U_ECALL:
        printf("User ecall\n");
        rv_mepc_inc();
        break;

    default:
        printf("Unknown trap (mcause = 0x%08X)\n", mcause);
        while (true) {
        }
    }
}
