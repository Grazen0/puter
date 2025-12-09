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

int _close([[maybe_unused]] const int file)
{
    return -1;
}

char *__env[1] = {0};
char **environ = __env;

int _execve([[maybe_unused]] char *const name, [[maybe_unused]] char **const argv,
            [[maybe_unused]] char **const env)
{
    errno = ENOMEM;
    return -1;
}

int _fork(void)
{
    errno = EAGAIN;
    return -1;
}

int _fstat([[maybe_unused]] const int file, [[maybe_unused]] struct stat *const st)
{
    st->st_mode = __S_IFCHR;
    return 0;
}

int _getpid(void)
{
    return 1;
}

int _isatty([[maybe_unused]] const int file)
{
    return 1;
}

int _kill([[maybe_unused]] const int pid, [[maybe_unused]] const int sig)
{
    errno = EINVAL;
    return -1;
}

int _link([[maybe_unused]] char *const old, [[maybe_unused]] char *const new)
{
    errno = EMLINK;
    return -1;
}

int _lseek([[maybe_unused]] const int file, [[maybe_unused]] const int ptr,
           [[maybe_unused]] const int dir)
{
    return 0;
}

int _open([[maybe_unused]] const char *name, [[maybe_unused]] const int flags,
          [[maybe_unused]] const int mode)
{
    return -1;
}

int _read([[maybe_unused]] const int file, [[maybe_unused]] char *const ptr,
          [[maybe_unused]] const int len)
{
    return 0;
}

void *_sbrk(const int incr)
{
    extern char _end;

    static char *heap_end = &_end;
    char *prev_heap_end = heap_end;

    register char *stack_ptr __asm__("sp");

    if (heap_end + incr > stack_ptr) {
        print("Heap and stack collision\n");
        while (true) {
        }
    }

    heap_end += incr;
    return prev_heap_end;
}

int stat([[maybe_unused]] const char *file, [[maybe_unused]] struct stat *const st)
{
    st->st_mode = __S_IFCHR;
    return 0;
}

clock_t times([[maybe_unused]] struct tms *const buf)
{
    return -1;
}

int unlink([[maybe_unused]] char *const name)
{
    errno = ENOENT;
    return -1;
}

int wait([[maybe_unused]] int *const status)
{
    errno = ECHILD;
    return -1;
}

int _write([[maybe_unused]] const int file, char *const ptr, const int len)
{
    sprint(ptr, len);
    return len;
}
