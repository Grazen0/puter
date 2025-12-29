#ifndef FIRMWARE_UART_H
#define FIRMWARE_UART_H

#include "numeric.h"
#include <stddef.h>

void uart_write(u8 byte);

void uart_print(const char s[]);

void uart_sprint(const char s[], size_t n);

#endif
