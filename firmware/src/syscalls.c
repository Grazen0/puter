#include "utils.h"
#include <errno.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <time.h>

#undef errno
extern int errno;

void _exit(void)
{
    while (true) {
    }
}

int _close(int file)
{
    return -1;
}

char *__env[1] = {0};
char **environ = __env;

int _execve(char *name, char **argv, char **env)
{
    errno = ENOMEM;
    return -1;
}

int _fork(void)
{
    errno = EAGAIN;
    return -1;
}

int _fstat(int file, struct stat *st)
{
    st->st_mode = __S_IFCHR;
    return 0;
}

int _getpid(void)
{
    return 1;
}

int _isatty(int file)
{
    return 1;
}

int _kill(int pid, int sig)
{
    errno = EINVAL;
    return -1;
}

int _link(char *old, char *new)
{
    errno = EMLINK;
    return -1;
}

int _lseek(int file, int ptr, int dir)
{
    return 0;
}

int _open(const char *name, int flags, int mode)
{
    return -1;
}

int _read(int file, char *ptr, int len)
{
    return 0;
}

void *_sbrk(int incr)
{
    extern char _end;
    extern char _stack_bottom;

    static char *heap_end = &_end;
    char *prev_heap_end = heap_end;

    register char *stack_ptr __asm__("sp");

    if (heap_end + incr > stack_ptr) {
        print("Heap and stack collision\n");
        while (true) {
        }
    }

    heap_end += incr;
    return (void *)prev_heap_end;
}

int stat(const char *file, struct stat *st)
{
    st->st_mode = __S_IFCHR;
    return 0;
}

clock_t times(struct tms *buf)
{
    return -1;
}

int unlink(char *name)
{
    errno = ENOENT;
    return -1;
}

int wait(int *status)
{
    errno = ECHILD;
    return -1;
}

int _write(int file, char *ptr, int len)
{
    sprint(ptr, len);
    return len;
}
