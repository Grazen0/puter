#ifndef FIRMWARE_RISCV_H
#define FIRMWARE_RISCV_H

#include "numeric.h"
#include <stdint.h>

typedef enum {
    MIE_SOFTWARE = 0x008,
    MIE_TIMER = 0x080,
    MIE_EXTERNAL = 0x800,
} MieField;

typedef enum {
    MSTATUS_MIE = 0x8,
} MStatusField;

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

char *rv_sp_read(void);

u32 rv_mstatus_read(void);

u64 rv_mcycle_read(void);

u32 rv_mcause_read(void);

uintptr_t rv_mepc_read(void);

void rv_mepc_inc(void);

void rv_mstatus_set(u32 n);

void rv_mie_set(u32 n);

[[noreturn]] void rv_jump_umode(void (*fn)(void));

#endif
