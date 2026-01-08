#include "uart.h"
#include "puter.h"

void uart_write(const u8 byte)
{
    while (!UART->ready) {
    }

    UART->out = byte;
}

void uart_print(const char s[])
{
    while (*s != '\0')
        uart_write(*s++);
}

void uart_sprint(const char s[], size_t n)
{
    while (n-- > 0)
        uart_write(*s++);
}
