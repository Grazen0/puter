#ifndef FIRMWARE_VGA_H
#define FIRMWARE_VGA_H

#include <stddef.h>

void vga_init();

void vga_clear();

void vga_putchar(char ch);

void vga_print(const char s[static 1]);

void vga_sprint(size_t n, const char s[static n + 1]);

#endif
