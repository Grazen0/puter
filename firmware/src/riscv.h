#ifndef FIRMWARE_RISCV_H
#define FIRMWARE_RISCV_H

#include "numeric.h"
#include <stdint.h>

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

char *read_sp(void);

u32 read_mstatus(void);

u64 read_mcycle(void);

u32 read_mcause(void);

uintptr_t read_mepc(void);

void inc_mepc(void);

void enable_mti(void);

#endif
