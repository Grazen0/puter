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
        out.rem = (out.rem << 1) | ((numer >> (LONG_WIDTH - 1)) & 1);
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
    PANIC_IF(base != 10, "strol supports base 10 only");

    long out = 0;

    bool neg = false;

    if (*nptr == '-') {
        ++nptr;
        neg = true;
    }

    while (is_digit(*nptr)) {
        const long digit = *nptr - '0';
        ++nptr;

        out *= 10;

        if (neg)
            out -= digit;
        else
            out += digit;
    }

    if (endptr != nullptr)
        *endptr = (char *)nptr;

    return out;
}

unsigned long strtoul(const char *restrict nptr, char **restrict endptr,
                      const int base)
{
    PANIC_IF(base != 10, "strtoul supports base 10 only");

    unsigned long out = 0;

    while (is_digit(*nptr)) {
        out = (10 * out) + (*nptr - '0');
        ++nptr;
    }

    if (endptr != nullptr)
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

int __umodsi3(unsigned a, const unsigned b)
{
    PANIC_IF(b == 0, "cannot calculate modulo by 0");

    unsigned rem = 0;

    for (size_t i = 0; i < LONG_WIDTH; ++i) {
        rem = (rem << 1) | ((a >> (LONG_WIDTH - 1)) & 1);
        a <<= 1;

        if (rem >= b) {
            rem -= b;
        }
    }

    return (int)rem;
}
