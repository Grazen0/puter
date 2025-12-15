#ifndef FIRMWARE_PUTER_H
#define FIRMWARE_PUTER_H

#include "numeric.h"
#include <stddef.h>

typedef struct {
    volatile char out;
} DebugControl;

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

constexpr size_t SCREEN_ROWS = 30;
constexpr size_t SCREEN_COLS = 80;
constexpr size_t TRAM_SIZE = SCREEN_ROWS * SCREEN_COLS;

constexpr size_t DBG_BASE = 0x1000'0000U;
constexpr size_t TRAM_BASE = 0xC000'0000U;
constexpr size_t RTC_BASE = 0xE000'0000U;
constexpr size_t KEYBOARD_BASE = 0xE800'0000U;
constexpr size_t PLIC_BASE = 0xF000'0000U;
constexpr size_t MEIID_BASE = 0xF800'0000U;

#define DBG ((DebugControl *)DBG_BASE)
#define TRAM ((volatile char *)TRAM_BASE)
#define RTC ((RealTimeCounter *)RTC_BASE)
#define KEYBOARD ((Keyboard *)KEYBOARD_BASE)
#define PLIC ((Plic *)PLIC_BASE)
#define MEIID (*(volatile u8 *)MEIID_BASE)

void init_display(void);

void print(const char *s);

void sprint(const char *s, size_t n);

void print_int(int n);

void print_hex(u32 n);

#endif
