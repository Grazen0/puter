#include "vga.h"
#include "control.h"
#include "libc.h"
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
    CharAttr_FgMask = 0x0F,

    CharAttr_BgBlack = 0x00,
    CharAttr_BgBlue = 0x10,
    CharAttr_BgGreen = 0x20,
    CharAttr_BgCyan = 0x30,
    CharAttr_BgRed = 0x40,
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
    CharAttr_BgMask = 0xF0,
} CharAttr;

typedef enum : u8 {
    State_Normal,
    State_Esc,
    State_Csi,
} State;

static constexpr u8 CH_ATTR_DEFAULT = CharAttr_FgWhite | CharAttr_BgBlack;
static constexpr u16 VVALUE_EMPTY = CH_ATTR_DEFAULT << 8;
static constexpr char SEQ_BUF_SIZE = 32;

typedef struct {
    char seq_buf[SEQ_BUF_SIZE];
    size_t seq_buf_idx;
    size_t tram_idx;
    TRamEntry base_value;
    State state;
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

static void process_csi_color_code(const unsigned n)
{
    constexpr long SET_FG_COLOR_BASE = 30;
    constexpr long SET_FG_COLOR_END = SET_FG_COLOR_BASE + 8;

    constexpr long SET_FG_COLOR_BRIGHT_BASE = 90;
    constexpr long SET_FG_COLOR_BRIGHT_END = SET_FG_COLOR_BRIGHT_BASE + 8;

    constexpr long SET_BG_COLOR_BASE = 40;
    constexpr long SET_BG_COLOR_END = SET_BG_COLOR_BASE + 8;

    constexpr long SET_BG_COLOR_BRIGHT_BASE = 100;
    constexpr long SET_BG_COLOR_BRIGHT_END = SET_BG_COLOR_BRIGHT_BASE + 8;

    static const CharAttr n_to_fg_attr[] = {
        CharAttr_FgBlack, CharAttr_FgRed,  CharAttr_FgGreen,
        CharAttr_FgBrown, CharAttr_FgBlue, CharAttr_FgMagenta,
        CharAttr_FgCyan,  CharAttr_FgGray,
    };

    static const CharAttr n_to_fg_attr_bright[] = {
        CharAttr_FgGray,        CharAttr_FgBrightRed,
        CharAttr_FgBrightGreen, CharAttr_FgBrightYellow,
        CharAttr_FgBrightBlue,  CharAttr_FgBrightMagenta,
        CharAttr_FgBrightCyan,  CharAttr_FgWhite,
    };

    static const CharAttr n_to_bg_attr[] = {
        CharAttr_BgBlack, CharAttr_BgRed,  CharAttr_BgGreen,
        CharAttr_BgBrown, CharAttr_BgBlue, CharAttr_BgMagenta,
        CharAttr_BgCyan,  CharAttr_BgGray,
    };

    static const CharAttr n_to_bg_attr_bright[] = {
        CharAttr_BgGray,        CharAttr_BgBrightRed,
        CharAttr_BgBrightGreen, CharAttr_BgBrightYellow,
        CharAttr_BgBrightBlue,  CharAttr_BgBrightMagenta,
        CharAttr_BgBrightCyan,  CharAttr_BgWhite,
    };

    if (n >= SET_FG_COLOR_BASE && n < SET_FG_COLOR_END) {
        ctx.base_value.attr &= ~CharAttr_FgMask;
        ctx.base_value.attr |= n_to_fg_attr[n - SET_FG_COLOR_BASE];
    } else if (n >= SET_FG_COLOR_BRIGHT_BASE && n < SET_FG_COLOR_BRIGHT_END) {
        ctx.base_value.attr &= ~CharAttr_FgMask;
        ctx.base_value.attr |=
            n_to_fg_attr_bright[n - SET_FG_COLOR_BRIGHT_BASE];
    } else if (n >= SET_BG_COLOR_BASE && n < SET_BG_COLOR_END) {
        ctx.base_value.attr &= ~CharAttr_BgMask;
        ctx.base_value.attr |= n_to_bg_attr[n - SET_BG_COLOR_BASE];
    } else if (n >= SET_BG_COLOR_BRIGHT_BASE && n < SET_BG_COLOR_BRIGHT_END) {
        ctx.base_value.attr &= ~CharAttr_BgMask;
        ctx.base_value.attr |=
            n_to_bg_attr_bright[n - SET_BG_COLOR_BRIGHT_BASE];
    }
}

static void process_csi(const char final_byte)
{

    switch (final_byte) {
    case 'A': // Cursor up
        if (ctx.tram_idx >= SCREEN_COLS)
            ctx.tram_idx -= SCREEN_COLS;

        update_cursor_pos();
        break;

    case 'B': // Cursor down
        if (ctx.tram_idx < TRAM_SIZE - SCREEN_COLS)
            ctx.tram_idx += SCREEN_COLS;

        update_cursor_pos();
        break;

    case 'C': // Cursor forward
        if (((ctx.tram_idx + 1) % SCREEN_COLS) != 0)
            ++ctx.tram_idx;

        update_cursor_pos();
        break;

    case 'D': // Cursor backward
        if ((ctx.tram_idx % SCREEN_COLS) != 0)
            --ctx.tram_idx;

        update_cursor_pos();
        break;

    case 'm':
        const char *ptr = ctx.seq_buf;

        // Note that ctx.seq_buf is null-terminated, which helps a lot.
        while (true) {
            char *end = nullptr;
            const unsigned n = strtoul(ptr, &end, 10);

            if (n == 0)
                ctx.base_value.attr = CH_ATTR_DEFAULT;
            else
                process_csi_color_code(n);

            ptr = end;

            if (*ptr == '\0')
                break;

            if (*ptr == ';')
                ++ptr;

            if (!is_digit(*ptr))
                break;
        }
        break;

    default:
        break;
    }
}

static void vga_putchar_inner(const char ch)
{
#ifndef NDEBUG
    DBG->out = ch;
#endif

    switch (ctx.state) {
    case State_Normal:
        switch (ch) {
        case '\n':
            ctx.tram_idx += SCREEN_COLS - (ctx.tram_idx % SCREEN_COLS);
            break;

        case '\r':
            ctx.tram_idx -= (ctx.tram_idx % SCREEN_COLS);
            break;

        case '\t':
            constexpr size_t TAB_WIDTH = 4;

            do {
                TRAM[ctx.tram_idx++].value = VVALUE_EMPTY;
            } while ((ctx.tram_idx % TAB_WIDTH) != 0);
            break;

        case '\033': // ESC
            ctx.state = State_Esc;
            break;

        default:
            TRAM[ctx.tram_idx++].value = ctx.base_value.value | ch;
        }

        if (ctx.tram_idx >= TRAM_SIZE) {
            scroll();
            ctx.tram_idx = (SCREEN_ROWS - 1) * SCREEN_COLS;
        }
        break;

    case State_Esc:
        if (ch == '[') {
            ctx.state = State_Csi;
            ctx.seq_buf_idx = 0;
            ctx.seq_buf[0] = '\0';
        } else {

            ctx.state = State_Normal;
        }
        break;

    case State_Csi:
        if (ch >= 0x40 && ch <= 0x7E) {
            process_csi(ch);
            ctx.state = State_Normal;
        } else {
            PANIC_IF(ctx.seq_buf_idx >= SEQ_BUF_SIZE - 1,
                     "VGA sequence buffer overflow");
            ctx.seq_buf[ctx.seq_buf_idx++] = ch;
            ctx.seq_buf[ctx.seq_buf_idx] = '\0';
        }
        break;

    default:
        PANIC("invalid vga state (state = %u)", ctx.state);
    }
}

void vga_init()
{
    ctx = (VgaContext){
        .tram_idx = 0,
        .base_value.value = VVALUE_EMPTY,
        .state = State_Normal,
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

void(vga_sprint)(size_t n, const char s[static n])
{
    while (n-- != 0)
        vga_putchar_inner(*s++);

    update_cursor_pos();
}
