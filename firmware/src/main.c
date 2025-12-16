#include "numeric.h"
#include "puter.h"
#include "riscv.h"
#include <stddef.h>
#include <stdio.h>

static constexpr char banner[] = "\
 ____        _             ___  ____   \n\
|  _ \\ _   _| |_ ___ _ __ / _ \\/ ___|  \n\
| |_) | | | | __/ _ \\ '__| | | \\___ \\  \n\
|  __/| |_| | ||  __/ |  | |_| |___) | \n\
|_|    \\__,_|\\__\\___|_|   \\___/|____/  \n\
";

static constexpr size_t MTI_FREQ = 1000;

static constexpr size_t SCANCODES_CAPACITY = 32;

static volatile u8 scancodes[SCANCODES_CAPACITY];
static size_t scancodes_head = 0;
static volatile size_t scancodes_tail = 0;

static inline bool scancode_available(void)
{
    return scancodes_head != scancodes_tail;
}

static inline u8 scancode_take(void)
{
    const u8 scancode = scancodes[scancodes_head];

    if (++scancodes_head >= SCANCODES_CAPACITY)
        scancodes_head = 0;

    return scancode;
}

static volatile u32 ticks = 0;

void sleep_ms(const u32 ms)
{
    const u32 start = ticks;
    const u32 end = start + ms;

    if (end < start) {
        while (ticks > start) {
        }
    }

    while (ticks < end) {
    }
}

void main(void)
{
    init_display();

    print("Initializing RTC...\n");

    RTC->mtime = 0;
    RTC->mtimecmp = RTC_FREQ / MTI_FREQ;

    printf("Initializing PLIC...\n");

    for (size_t i = 0; i < PLIC_PORTS; ++i) {
        PLIC->int_enable[i] = 1;
        PLIC->int_priority[i] = 1 + i;
    }

    printf("Enabling interrupts...\n");
    enable_mti();

    printf("\n");

    printf("Wake up, Neo...\n");
    printf("\n");
    printf("%s", banner);
    printf("\n");
    printf("Welcome to PuterOS.\n");
    printf("\n");

    while (true) {
        sleep_ms(500);

        while (scancode_available()) {
            printf("%02X ", scancode_take(), scancodes_head, scancodes_tail);
            fflush(0);
        }
    }
}

typedef enum : u8 {
    MEIID_KEYBOARD = 0,
} MeiId;

[[gnu::interrupt]] void trap_handler(void)
{
    // TODO: set up reentrancy

    const u32 mcause = read_mcause();

    switch (mcause) {
    case MCAUSE_M_TIMER_INT:
        RTC->mtime = 0;
        ++ticks;
        break;

    case MCAUSE_M_EXTERNAL_INT:
        const u8 int_id = MEIID;
        PLIC->int_claim[int_id] = 1;

        if (int_id == MEIID_KEYBOARD) {
            scancodes[scancodes_tail] = KEYBOARD->scancode;

            if (++scancodes_tail >= SCANCODES_CAPACITY)
                scancodes_tail = 0;
        }

        break;

    case MCAUSE_ILLEGAL_INSTR:
        printf("Illegal instruction (pc = 0x%08X)\n", read_mepc());
        while (true) {
        }

    case MCAUSE_U_ECALL:
        printf("User ecall\n");
        inc_mepc();
        break;

    default:
        printf("Unknown trap (mcause = 0x%08X)\n", mcause);
        while (true) {
        }
    }
}
