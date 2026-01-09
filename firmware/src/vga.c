#include "vga.h"
#include "numeric.h"
#include "puter.h"
#include <stddef.h>
#include <stdlib.h>

typedef enum : u8 {
    CharAttr_FgBlack = 0x00,
    CharAttr_FgBlue = 0x01,
    CharAttr_FgGreen = 0x02,
    CharAttr_FgCyan = 0x03,
    CharAttr_FgRed = 0x04,
    CharAttr_FgMagenta = 0x05,
    CharAttr_FgBrown = 0x06,
    CharAttr_FgGray = 0x07,
    CharAttr_FgDarkGray = 0x08,
    CharAttr_FgBrightBlue = 0x09,
    CharAttr_FgBrightGreen = 0x0A,
    CharAttr_FgBrightCyan = 0x0B,
    CharAttr_FgBrightRed = 0x0C,
    CharAttr_FgBrightMagenta = 0x0D,
    CharAttr_FgBrightYellow = 0x0E,
    CharAttr_FgWhite = 0x0F,

    CharAttr_BgBlack = 0x00,
    CharAttr_BgBlue = 0x10,
    CharAttr_BgGreen = 0x20,
    CharAttr_BgCyan = 0x30,
    CharAttr_BgRED = 0x40,
    CharAttr_BgMagenta = 0x50,
    CharAttr_BgBrown = 0x60,
    CharAttr_BgGray = 0x70,
    CharAttr_BgDarkGray = 0x80,
    CharAttr_BgBrightBlue = 0x90,
    CharAttr_BgBrightGreen = 0xA0,
    CharAttr_BgBrightCyan = 0xB0,
    CharAttr_BgBrightRed = 0xC0,
    CharAttr_BgBrightMagenta = 0xD0,
    CharAttr_BgBrightYellow = 0xE0,
    CharAttr_BgWhite = 0xF0,
} CharAttr;

static constexpr u16 VVALUE_EMPTY = (CharAttr_FgWhite | CharAttr_BgBlack) << 8;

typedef struct {
    size_t tram_idx;
    u16 base_value;
} VgaContext;

static VgaContext ctx;

static inline void scroll()
{
    for (size_t i = 0; i < SCREEN_ROWS - 1; ++i) {
        for (size_t j = 0; j < SCREEN_COLS; ++j)
            TRAM[(i * SCREEN_COLS) + j] = TRAM[((i + 1) * SCREEN_COLS) + j];
    }

    for (size_t j = 0; j < SCREEN_COLS; ++j)
        TRAM[((SCREEN_ROWS - 1) * SCREEN_COLS) + j].value = VVALUE_EMPTY;
}

static inline void update_cursor_pos()
{
    VREGS->cursor_pos = ctx.tram_idx;
}

static void vga_putchar_inner(const char ch)
{
#ifndef NDEBUG
    DBG->out = ch;
#endif

    const u16 value = ctx.base_value | ch;

    switch (ch) {
    case '\n':
        ctx.tram_idx += SCREEN_COLS - mod_naive(ctx.tram_idx, SCREEN_COLS);
        break;

#ifdef PUTCHAR_FULL
    case '\r':
        ctx.tram_idx -= mod_naive(ctx.tram_idx, SCREEN_COLS);
        break;

    case '\t':
        constexpr size_t TAB_WIDTH = 4;

        do {
            TRAM[ctx.tram_idx++].value = VVALUE_EMPTY;
        } while (mod_naive(ctx.tram_idx, TAB_WIDTH) != 0);
        break;
#endif

    default:
        TRAM[ctx.tram_idx++].value = value;
    }

    if (ctx.tram_idx >= TRAM_SIZE) {
        scroll();
        ctx.tram_idx = (SCREEN_ROWS - 1) * SCREEN_COLS;
    }
}

void vga_init()
{
    ctx = (VgaContext){
        .tram_idx = 0,
        .base_value = VVALUE_EMPTY,
    };

    vga_clear();
}

void vga_clear()
{
    ctx.tram_idx = 0;

    for (size_t i = 0; i < TRAM_SIZE; ++i)
        TRAM[i].value = VVALUE_EMPTY;

    update_cursor_pos();
}

void vga_putchar(const char ch)
{
    vga_putchar_inner(ch);
    update_cursor_pos();
}

void vga_print(const char s[static 1])
{
    while (*s != '\0')
        vga_putchar_inner(*s++);

    update_cursor_pos();
}

#define vga_sprint(s, n) vga_sprint(n, s)

void(vga_sprint)(size_t n, const char s[static n])
{
    while (n-- != 0)
        vga_putchar_inner(*s++);

    update_cursor_pos();
}
