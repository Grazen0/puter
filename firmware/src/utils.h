#ifndef FIRMWARE_UTILS_H
#define FIRMWARE_UTILS_H

#include <stddef.h>

static constexpr size_t SCREEN_BASE = 0x1000'0000U;
#define SCREEN (*(volatile char *)SCREEN_BASE)

void print(const char *s);

void sprint(const char *s, size_t n);

#endif
