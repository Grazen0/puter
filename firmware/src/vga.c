#include "vga.h"
#include "numeric.h"
#include "puter.h"
#include <stddef.h>
#include <stdlib.h>

static constexpr size_t TAB_WIDTH = 4;
static constexpr u16 VVALUE_EMPTY = 0;

typedef enum : u8 {
    ATTR_FG_BLACK = 0x00,
    ATTR_FG_BLUE = 0x01,
    ATTR_FG_GREEN = 0x02,
    ATTR_FG_CYAN = 0x03,
    ATTR_FG_RED = 0x04,
    ATTR_FG_MAGENTA = 0x05,
    ATTR_FG_BROWN = 0x06,
    ATTR_FG_GRAY = 0x07,
    ATTR_FG_DARK_GRAY = 0x08,
    ATTR_FG_BRIGHT_BLUE = 0x09,
    ATTR_FG_BRIGHT_GREEN = 0x0A,
    ATTR_FG_BRIGHT_CYAN = 0x0B,
    ATTR_FG_BRIGHT_RED = 0x0C,
    ATTR_FG_BRIGHT_MAGENTA = 0x0D,
    ATTR_FG_BRIGHT_YELLOW = 0x0E,
    ATTR_FG_WHITE = 0x0F,

    ATTR_BG_BLACK = 0x00,
    ATTR_BG_BLUE = 0x10,
    ATTR_BG_GREEN = 0x20,
    ATTR_BG_CYAN = 0x30,
    ATTR_BG_RED = 0x40,
    ATTR_BG_MAGENTA = 0x50,
    ATTR_BG_BROWN = 0x60,
    ATTR_BG_GRAY = 0x70,
    ATTR_BG_DARK_GRAY = 0x80,
    ATTR_BG_BRIGHT_BLUE = 0x90,
    ATTR_BG_BRIGHT_GREEN = 0xA0,
    ATTR_BG_BRIGHT_CYAN = 0xB0,
    ATTR_BG_BRIGHT_RED = 0xC0,
    ATTR_BG_BRIGHT_MAGENTA = 0xD0,
    ATTR_BG_BRIGHT_YELLOW = 0xE0,
    ATTR_BG_WHITE = 0xF0,
} TextAttr;

typedef struct {
    size_t tram_idx;
    u16 base_value;
} VgaContext;

static VgaContext ctx;

static inline void scroll(void)
{
    for (size_t i = 0; i < SCREEN_ROWS - 1; ++i) {
        for (size_t j = 0; j < SCREEN_COLS; ++j)
            TRAM[(i * SCREEN_COLS) + j] = TRAM[((i + 1) * SCREEN_COLS) + j];
    }

    for (size_t j = 0; j < SCREEN_COLS; ++j)
        TRAM[((SCREEN_ROWS - 1) * SCREEN_COLS) + j].value = VVALUE_EMPTY;
}

void vga_init(void)
{
    ctx = (VgaContext){
        .tram_idx = 0,
        .base_value = (u16)(ATTR_FG_WHITE | ATTR_BG_BLACK) << 8,
    };

    vga_clear();
}

void vga_clear(void)
{
    ctx.tram_idx = 0;

    for (size_t i = 0; i < TRAM_SIZE; ++i)
        TRAM[i].value = VVALUE_EMPTY;
}

void vga_print_char(const char ch)
{
#ifdef DEBUG
    DBG->out = ch;
#endif

    const u16 value = ctx.base_value | ch;

    switch (ch) {
    case '\n':
        ctx.tram_idx += SCREEN_COLS - (ctx.tram_idx % SCREEN_COLS);
        break;

    case '\r':
        ctx.tram_idx -= ctx.tram_idx % SCREEN_COLS;
        break;

    case '\t':
        do {
            TRAM[ctx.tram_idx++].value = VVALUE_EMPTY;
        } while ((ctx.tram_idx % TAB_WIDTH) != 0);
        break;

    default:
        TRAM[ctx.tram_idx++].value = value;
    }

    if (ctx.tram_idx >= TRAM_SIZE) {
        scroll();
        ctx.tram_idx = (SCREEN_ROWS - 1) * SCREEN_COLS;
    }
}

void vga_print(const char *s)
{
    while (*s != '\0')
        vga_print_char(*s++);
}

void vga_sprint(const char *s, size_t n)
{
    while (n-- != 0)
        vga_print_char(*s++);
}
