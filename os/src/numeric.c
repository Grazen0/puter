#include "numeric.h"

size_t mod_naive(size_t a, const size_t b)
{
    while (a >= b)
        a -= b;

    return a;
}
