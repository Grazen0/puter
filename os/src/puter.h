#ifndef FIRMWARE_PUTER_H
#define FIRMWARE_PUTER_H

#include "numeric.h"
#include <stddef.h>
#include <stdint.h>

constexpr size_t SYS_CLK_FREQ = 50'000'000U;

typedef struct {
    volatile char out;
} DebugControl;

typedef union {
    u16 value;
    struct {
        char ch;
        u8 attr;
    };
} TRamEntry;

typedef struct {
    volatile bool cursor_enabled;
    const u8 _padding_0;
    volatile u16 cursor_pos;
    volatile u8 cursor_start_scanline;
    const u8 _padding_1;
    volatile u8 cursor_end_scanline;
} VgaRegisters;

typedef union {
    volatile const bool ready;
    volatile u8 out;
} Uart;

typedef union {
    struct {
        volatile const bool ready;
        volatile const bool rdata_valid;
        volatile const u8 rdata;
    };
    struct {
        volatile u8 cs;
        volatile u8 sclk_half_period;
        volatile u8 transfer;
    };
} Spi;

constexpr size_t RTC_FREQ = 100'000'000U;

typedef struct {
    volatile u64 mtime;
    volatile u64 mtimecmp;
} RealTimeCounter;

typedef struct {
    volatile const u8 scancode;
} Keyboard;

constexpr size_t PLIC_PORTS = 2;

typedef struct {
    volatile u8 int_priority[PLIC_PORTS];
    volatile bool int_enable[PLIC_PORTS];
    volatile u8 int_claim[PLIC_PORTS];
} Plic;

typedef enum : u8 {
    MeiId_Keyboard = 0,
} MeiId;

constexpr size_t SCREEN_ROWS = 30;
constexpr size_t SCREEN_COLS = 80;
constexpr size_t TRAM_SIZE = SCREEN_ROWS * SCREEN_COLS;

constexpr uintptr_t DBG_BASE = 0x0000'FFFFU;
constexpr uintptr_t TRAM_BASE = 0x0800'0000U;
constexpr uintptr_t VREGS_BASE = 0x1000'0000U;
constexpr uintptr_t UART_BASE = 0x1800'0000U;
constexpr uintptr_t SPI_BASE = 0x2000'0000U;
constexpr uintptr_t RTC_BASE = 0x2800'0000U;
constexpr uintptr_t KEYBOARD_BASE = 0x3000'0000U;
constexpr uintptr_t PLIC_BASE = 0x3800'0000U;
constexpr uintptr_t MEIID_BASE = 0x4000'0000U;

#define DBG ((DebugControl *)DBG_BASE)
#define TRAM ((TRamEntry *)TRAM_BASE)
#define VREGS ((VgaRegisters *)VREGS_BASE)
#define UART ((Uart *)UART_BASE)
#define SPI ((Spi *)SPI_BASE)
#define RTC ((RealTimeCounter *)RTC_BASE)
#define KEYBOARD ((Keyboard *)KEYBOARD_BASE)
#define PLIC ((Plic *)PLIC_BASE)
#define MEIID (*(volatile u8 *)MEIID_BASE)

#endif
