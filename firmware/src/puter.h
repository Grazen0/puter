#ifndef FIRMWARE_PUTER_H
#define FIRMWARE_PUTER_H

#include "numeric.h"
#include <stddef.h>

typedef struct {
    volatile char out;
} DebugControl;

typedef union {
    volatile u16 value;
    struct {
        volatile char ch;
        volatile u8 attr;
    };
} TRamEntry;

typedef struct {
    volatile bool cursor_enabled;
    u8 _padding_0;
    volatile u16 cursor_pos;
    volatile u8 cursor_start_scanline;
    u8 _padding_1;
    volatile u8 cursor_end_scanline;
} VideoRegisters;

typedef struct {
    union {
        volatile u8 out;
        volatile bool ready;
    };
} Uart;

constexpr size_t RTC_FREQ = 100'000'000U;

typedef struct {
    volatile u64 mtime;
    volatile u64 mtimecmp;
} RealTimeCounter;

typedef struct {
    volatile u8 scancode;
} Keyboard;

constexpr size_t PLIC_PORTS = 2;

typedef struct {
    volatile u8 int_priority[PLIC_PORTS];
    volatile bool int_enable[PLIC_PORTS];
    volatile u8 int_claim[PLIC_PORTS];
} Plic;

typedef enum : u8 {
    MEIID_KEYBOARD = 0,
} MeiId;

constexpr size_t SCREEN_ROWS = 30;
constexpr size_t SCREEN_COLS = 80;
constexpr size_t TRAM_SIZE = SCREEN_ROWS * SCREEN_COLS;

constexpr size_t DBG_BASE = 0x1000'0000U;
constexpr size_t TRAM_BASE = 0xC000'0000U;
constexpr size_t VREGS_BASE = 0xC800'0000U;
constexpr size_t UART_BASE = 0xD000'0000U;
constexpr size_t RTC_BASE = 0xE000'0000U;
constexpr size_t KEYBOARD_BASE = 0xE800'0000U;
constexpr size_t PLIC_BASE = 0xF000'0000U;
constexpr size_t MEIID_BASE = 0xF800'0000U;

#define DBG ((DebugControl *)DBG_BASE)
#define TRAM ((TRamEntry *)TRAM_BASE)
#define VREGS ((VideoRegisters *)VREGS_BASE)
#define UART ((Uart *)UART_BASE)
#define RTC ((RealTimeCounter *)RTC_BASE)
#define KEYBOARD ((Keyboard *)KEYBOARD_BASE)
#define PLIC ((Plic *)PLIC_BASE)
#define MEIID (*(volatile u8 *)MEIID_BASE)

#endif
