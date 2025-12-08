#include "utils.h"

void print(const char *s)
{
    while (*s != '\0')
        SCREEN = *s++;
}

void sprint(const char *s, size_t n)
{
    while (n-- != 0)
        SCREEN = *s++;
}
