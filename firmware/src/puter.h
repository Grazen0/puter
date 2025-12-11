#ifndef FIRMWARE_PUTER_H
#define FIRMWARE_PUTER_H

#include "numeric.h"
#include <stddef.h>

typedef struct {
    volatile char out;
} DebugControl;

static constexpr size_t SCREEN_ROWS = 30;
static constexpr size_t SCREEN_COLS = 80;
static constexpr size_t TRAM_SIZE = SCREEN_ROWS * SCREEN_COLS;

static constexpr size_t DBG_BASE = 0x1000'0000U;
static constexpr size_t TRAM_BASE = 0xC000'0000U;

#define DBG ((DebugControl *)DBG_BASE)
#define TRAM ((volatile char *)TRAM_BASE)

void init_display(void);

void print(const char *s);

void sprint(const char *s, size_t n);

void print_int(int n);

void print_hex(u32 n);

#endif
