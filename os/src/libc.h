#ifndef PUTEROS_LIBC_H
#define PUTEROS_LIBC_H

typedef struct {
    unsigned long quot;
    unsigned long rem;
} uldiv_t;

uldiv_t uldiv(unsigned long numer, unsigned long denom);

bool is_digit(char ch);

#endif
