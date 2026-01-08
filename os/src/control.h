#ifndef PUTEROS_CONTROL_h
#define PUTEROS_CONTROL_h

#include "io.h"

#define PANIC(...)                                             \
    do {                                                       \
        printk("PANIC (%s:%lld)\n", __FILE__, __LINE__ + 0LL); \
        __VA_OPT__(printk(__VA_ARGS__);)                       \
        while (true) {                                         \
        }                                                      \
    } while (false)

#define PANIC_IF(cond, ...)     \
    do {                        \
        if (cond)               \
            PANIC(__VA_ARGS__); \
    } while (false)

#endif
