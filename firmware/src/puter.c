#include "puter.h"
#include "numeric.h"
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>

static size_t tram_idx;

static constexpr size_t TAB_WIDTH = 4;

static inline void print_char(const char ch)
{
#ifdef DEBUG
    DBG->out = ch;
#endif

    switch (ch) {
    case '\n':
        tram_idx += SCREEN_COLS - (tram_idx % SCREEN_COLS);
        break;

    case '\r':
        tram_idx -= tram_idx % SCREEN_COLS;
        break;

    case '\t':
        do {
            TRAM[tram_idx++] = ' ';
        } while ((tram_idx % TAB_WIDTH) != 0);
        break;

    default:
        TRAM[tram_idx++] = ch;
    }
}

void init_display(void)
{
    tram_idx = 0;

    for (size_t i = 0; i < TRAM_SIZE; ++i)
        TRAM[i] = 0;
}

void print(const char *s)
{
    while (*s != '\0')
        print_char(*s++);
}

void sprint(const char *s, size_t n)
{
    while (n-- != 0)
        print_char(*s++);
}

void print_int(int n)
{
    if (n < 0)
        print_char('-');

    if (n == 0) {
        print_char('0');
        return;
    }

    static constexpr size_t MAX_DIGITS = 10;

    char digits[MAX_DIGITS + 1];

    char *start = &digits[MAX_DIGITS];
    *start = '\0';

    while (n != 0) {
        *(--start) = '0' + abs(n % 10);
        n /= 10;
    }

    print(start);
}

void print_hex(const u32 n)
{
    for (int i = 28; i >= 0; i -= 4) {
        const u8 nib = (n >> i) & 0xF;
        print_char(nib < 10 ? '0' + nib : 'A' + nib);
    }
}
