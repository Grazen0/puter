#ifndef PUTEROS_CONTROL_h
#define PUTEROS_CONTROL_h

#include "io.h"
#include "vga.h"

#define STRINGIFY(X) #X

#define STRGY(X) STRINGIFY(X)

#define PANIC(message)                                                        \
    do {                                                                      \
        vga_print("PANIC (" STRGY(__FILE__) ":" STRGY(__LINE__) "): " message \
                                                                "\n");        \
        while (true) {                                                        \
        }                                                                     \
    } while (false)

#define PANIC_IF(cond, ...)     \
    do {                        \
        if (cond)               \
            PANIC(__VA_ARGS__); \
    } while (false)

#endif
