#ifndef FIRMWARE_VGA_H
#define FIRMWARE_VGA_H

#include <stddef.h>

void vga_init(void);

void vga_clear(void);

void vga_print_char(char ch);

void vga_print(const char *s);

void vga_sprint(const char *s, size_t n);

#endif
