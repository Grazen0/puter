#ifndef FIRMWARE_RISCV_H
#define FIRMWARE_RISCV_H

#include "numeric.h"
#include <stdint.h>

typedef enum {
    MieBit_Software = 0x008,
    MieBit_Timer = 0x080,
    MieBit_External = 0x800,
} MieBit;

typedef enum {
    MStatus_Mie = 0x8,
} MStatusField;

typedef enum : u32 {
    MCause_Interrupt = 0x8000'0000,
    MCause_Exception = 0x0000'0000,
} MCauseType;

typedef enum : u32 {
    MCause_IllegalInstr = MCause_Exception | 2,
    MCause_Breakpoint = MCause_Exception | 3,
    MCause_UEcall = MCause_Exception | 8,
    MCause_MEcall = MCause_Exception | 11,
    MCause_MSoftwareInt = MCause_Interrupt | 3,
    MCause_MTimerInt = MCause_Interrupt | 7,
    MCause_MExternalInt = MCause_Interrupt | 11,
} MCause;

char *rv_read_sp();

u32 rv_read_mstatus();

u64 rv_read_mcycle();

u64 rv_read_minstret();

u32 rv_read_mcause();

uintptr_t rv_read_mepc();

void rv_inc_mepc();

void rv_set_mstatus(u32 n);

void rv_set_mie(u32 n);

[[noreturn]] void rv_jump_umode(void fn());

#endif
