#ifndef FIRMWARE_CONTROL_H
#define FIRMWARE_CONTROL_H

#include <stdio.h>

#define PANIC(...)                                     \
    do {                                               \
        printf("PANIC (%s:%d)\n", __FILE__, __LINE__); \
        __VA_OPT__(printf(__VA_ARGS__);)               \
        while (true) {                                 \
        }                                              \
    } while (false)

#endif
