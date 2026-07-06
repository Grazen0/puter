#ifndef FIRMWARE_RISCV_H
#define FIRMWARE_RISCV_H

#include "numeric.h"

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

#endif
