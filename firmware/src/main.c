#include <stddef.h>
#include <stdio.h>

int fib(int n)
{
    if (n <= 1)
        return n;

    return fib(n - 1) + fib(n - 2);
}

void main(void)
{
    printf("Hello, world!\n");
    printf("fib(12) = %i\n", fib(12));
}
