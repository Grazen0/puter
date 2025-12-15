#ifndef FIRMWARE_RISCV_H
#define FIRMWARE_RISCV_H

#include "numeric.h"
#include <stdint.h>

char *read_sp(void);

u32 read_mstatus(void);

u64 read_mcycle(void);

u32 read_mcause(void);

uintptr_t read_mepc(void);

void inc_mepc(void);

void enable_mti(void);

#endif
