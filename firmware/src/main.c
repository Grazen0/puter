#include "puter.h"
#include "riscv.h"
#include <stddef.h>

static constexpr char banner[] = "\
 ____        _             ___  ____   \n\
|  _ \\ _   _| |_ ___ _ __ / _ \\/ ___|  \n\
| |_) | | | | __/ _ \\ '__| | | \\___ \\  \n\
|  __/| |_| | ||  __/ |  | |_| |___) | \n\
|_|    \\__,_|\\__\\___|_|   \\___/|____/  \n\
";

void main(void)
{
    init_display();

    print("Wake up, Neo...\n");
    print("\n");
    print(banner);
    print("\n");
    print("Welcome to PuterOS.\n");
}

typedef enum {
    MCAUSE_ILLEGAL_INSTR = 2,
    MCAUSE_BREAKPOINT = 3,
    MCAUSE_U_ECALL = 8,
    MCAUSE_M_ECALL = 11,
} ExceptionCause;

[[gnu::interrupt]] void trap_handler(void)
{
    const int mcause = read_mcause();
    const bool interrupt = (mcause & 0x8000'0000) != 0;
    const int excode = mcause & 0x7FFF'FFFF;

    if (interrupt) {
        print("interrupt (excode = ");
        print_int(excode);
        print(")\n");

        __asm__("csrc mip, 0x8");
        return;
    }

    switch (excode) {
    case MCAUSE_ILLEGAL_INSTR:
        print("Illegal instruction (pc = ");
        print_hex(read_mepc());
        print(")\n");

        while (true) {
        }

    case MCAUSE_BREAKPOINT:
        print("Breakpoint (pc = ");
        print_hex(read_mepc());
        print(")\n");
        while (true) {
        }

    case MCAUSE_U_ECALL:
        print("User ecall\n");
        inc_mepc();
        break;

    case MCAUSE_M_ECALL:
        print("Machine ecall\n");
        inc_mepc();
        break;

    default:
        print("Unknown exception (excode = ");
        print_int(excode);
        print(")\n");
        while (true) {
        }
    }
}
