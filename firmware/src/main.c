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
static volatile size_t scancodes_head = 0;
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

static u32 count_thing = 0;

void main(void)
{
    init_display();

    print("Initializing RTC...\n");

    RTC->mtime = 0;
    RTC->mtimecmp = RTC_FREQ / MTI_FREQ;

    print("Initializing PLIC...\n");

    for (size_t i = 0; i < PLIC_PORTS; ++i) {
        PLIC->int_enable[i] = 1;
        PLIC->int_priority[i] = 1 + i;
    }

    print("Enabling interrupts...\n");
    enable_mti();

    print("\n");

    print("Wake up, Neo...\n");
    print("\n");
    print(banner);
    print("\n");
    print("Welcome to PuterOS.\n");
    print("\n");

    while (true) {
        printf("head = %u, tail = %u, mstatus = 0x%08X\n", scancodes_head, scancodes_tail,
               read_mstatus());

        while (scancode_available())
            printf("%02X (head = %u, tail = %u)\n", scancode_take(), scancodes_head,
                   scancodes_tail);

        // static u32 count = 0;

        // print_int(++count);
        // print(" ");
        // print_int(count_thing);
        // print(" hello (mstatus = ");
        // print_hex(read_mstatus());
        // print(")\n");
    }
}

typedef enum : u32 {
    MCAUSE_INTERRUPT = 0x8000'0000,
    MCAUSE_EXCEPTION = 0x0000'0000,
} MCauseType;

typedef enum : u32 {
    MCAUSE_ILLEGAL_INSTR = MCAUSE_EXCEPTION | 2,
    MCAUSE_BREAKPOINT = MCAUSE_EXCEPTION | 3,
    MCAUSE_U_ECALL = MCAUSE_EXCEPTION | 8,
    MCAUSE_M_ECALL = MCAUSE_EXCEPTION | 11,
    MCAUSE_M_SOFTWARE_INT = MCAUSE_INTERRUPT | 3,
    MCAUSE_M_TIMER_INT = MCAUSE_INTERRUPT | 7,
    MCAUSE_M_EXTERNAL_INT = MCAUSE_INTERRUPT | 11,
} MCause;

typedef enum : u8 {
    MEIID_KEYBOARD = 0,
} MeiId;

[[gnu::interrupt]] void trap_handler(void)
{
    // TODO: set up reentrancy

    const u32 mcause = read_mcause();

    switch (mcause) {
    case MCAUSE_M_TIMER_INT:
        // TODO: check time till last timer interrupt with mcycle here
        // maybe a very short interval between interrupts causes the bug
        RTC->mtime = 0;
        ++ticks;
        break;

    case MCAUSE_M_EXTERNAL_INT:
        u64 mcycle = read_mcycle();

        const u8 int_id = MEIID;
        PLIC->int_claim[int_id] = 1;

        if (int_id == MEIID_KEYBOARD) {
            scancodes[scancodes_tail] = KEYBOARD->scancode;

            if (++scancodes_tail >= SCANCODES_CAPACITY)
                scancodes_tail = 0;

            count_thing++;

            static u64 last_mcycle = 0;

            print("keyboard interrupt (delay = ");
            print_int(mcycle - last_mcycle);
            print(")\n");

            last_mcycle = read_mcycle();
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
