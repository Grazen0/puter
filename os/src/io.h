#ifndef PUTEROS_IO_H
#define PUTEROS_IO_H

#include <stdarg.h>

void vprintk(const char format[static restrict 1], va_list args);

[[gnu::format(printf, 1, 2)]] void printk(const char format[static restrict 1],
                                          ...);

#endif
