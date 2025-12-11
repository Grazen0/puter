#ifndef FIRMWARE_RISCV_H
#define FIRMWARE_RISCV_H

char *read_sp(void);

int read_mcycle(void);

int read_mcause(void);

int read_mepc(void);

void inc_mepc(void);

#endif
