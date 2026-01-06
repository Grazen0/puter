#include "control.h"
#include "keyboard.h"
#include "numeric.h"
#include "puter.h"
#include "riscv.h"
#include "rtc.h"
#include "sd_card.h"
#include "spi.h"
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
    __asm__ volatile("csrr t0, mepc"); // should trigger illegal instruction exception
    PANIC("should not have reached here");
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

    printf("Initializing SD card...\n");
    const SdInitResult sd_result = sd_init();

    if (sd_result != SD_INIT_OK) {
        const char *const msg = sd_init_result_str(sd_result);
        printf("Could not initialize SD card: %s (code = %d)\n", msg, sd_result);
    }

    printf("Enabling interrupts...\n");
    rv_mie_set(MIE_TIMER | MIE_EXTERNAL);
    rv_mstatus_set(MSTATUS_MIE);

    printf("\n");

    const u32 mcycle = rv_read_mcycle();
    const u32 minstret = rv_read_minstret();
    printf("mcycle = %d, minstret = %d\n", mcycle, minstret);

    printf("Wake up, Neo...\n");
    printf("\n");
    printf("%s", banner);
    printf("\n");
    printf("Welcome to PuterOS.\n");
    printf("\n");

    for (u8 i = 0; i < 16; ++i)
        TRAM[i].attr = i << 4;

    sd_read_block(0);

    while (true) {
        kb_process_queue();

        Key key = {};

        while (kb_poll_key(&key))
            printf("key: %i, mod: %08X\n", key.code, key.mod);
    }
}

[[gnu::interrupt]] void trap_handler()
{
    const u32 mcause = rv_read_mcause();

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
        PANIC("Illegal instruction (pc = 0x%08X)\n", rv_read_mepc());

    case MCAUSE_U_ECALL:
        printf("User ecall\n");
        rv_mepc_inc();
        break;

    default:
        PANIC("Unknown trap (mcause = 0x%08X)\n", mcause);
    }
}
