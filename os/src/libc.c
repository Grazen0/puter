#include "libc.h"
#include "control.h"
#include "numeric.h"
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

bool is_digit(const char ch)
{
    return ch >= '0' && ch <= '9';
}

uldiv_t uldiv(unsigned long numer, const unsigned long denom)
{
    PANIC_IF(denom == 0, "cannot divide by 0");

    uldiv_t out = {
        .quot = 0,
        .rem = 0,
    };

    for (size_t i = 0; i < LONG_WIDTH; ++i) {
        out.rem = (out.rem << 1) | (((numer >> (LONG_WIDTH - 1)) & 1));
        numer <<= 1;
        out.quot <<= 1;

        if (out.rem >= denom) {
            out.quot += 1;
            out.rem -= denom;
        }
    }

    return out;
}

void *memcpy(void *const restrict dest, const void *const restrict src,
             size_t n)
{
    u8 *dest_b = dest;
    const u8 *src_b = src;

    while (n--)
        *dest_b++ = *src_b++;

    return dest;
}

void *memset(void *const s, const int c, size_t n)
{
    u8 *s_b = s;

    while (n--)
        *s_b = c;

    return s;
}

int abs(const int x)
{
    return x >= 0 ? x : -x;
}

long strtol(const char *restrict nptr, char **restrict endptr, const int base)
{
    PANIC_IF(base != 10, "supports base 10 only");

    long out = 0;

    while (is_digit(*nptr)) {
        out = (10 * out) + *nptr - '0';
        ++nptr;
    }

    *endptr = (char *)nptr;

    return out;
}

ldiv_t ldiv(const long numer, const long denom)
{
    PANIC_IF(numer == LONG_MAX && denom == -1, "cannot divide LONG_MAX / -1");

    auto const unumer =
        numer < 0 ? -(unsigned long)numer : (unsigned long)numer;
    auto const udenom =
        denom < 0 ? -(unsigned long)denom : (unsigned long)denom;

    const uldiv_t res = uldiv(unumer, udenom);

    return (ldiv_t){
        .quot = (numer < 0) ^ (denom < 0) ? -(long)res.quot : (long)res.quot,
        .rem = numer < 0 ? -(long)res.rem : (long)res.rem,
    };
}
