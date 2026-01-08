#include "numeric.h"
#include <stddef.h>

size_t mul(size_t a, size_t b)
{
    size_t prod = 0;

    while (b != 0) {
        if ((b & 1) != 0)
            prod += a;

        b >>= 1;
        a <<= 1;
    }

    return prod;
}

size_t mod_naive(size_t a, const size_t b)
{
    while (a >= b)
        a -= b;

    return a;
}
