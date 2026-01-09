#include "io.h"
#include "libc.h"
#include "vga.h"
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct {
    unsigned min_width;
    unsigned precision;
    char pad_char;
} PrintCtx;

static inline void print_ctx_init(PrintCtx ctx[static const 1])
{
    *ctx = (PrintCtx){
        .min_width = 0,
        .precision = UINT_MAX,
        .pad_char = ' ',
    };
}

// See
// https://dev.to/pauljlucas/determining-the-maximum-decimal-digits-at-compile-time-40lb
#define MAX_DEC_DIGITS(type) (((sizeof(type) * CHAR_BIT * 1233) >> 12) + 1)

static void print_l(const long n, const PrintCtx ctx[static const 1])
{
    static constexpr size_t BUF_SIZE = MAX_DEC_DIGITS(n);
    char buf[BUF_SIZE + 1]; // Extra char for sign

    size_t i = 0;

    if (n == 0) {
        buf[i++] = '0';
    } else {
        long n_tmp = n;

        while (n_tmp != 0) {
            const ldiv_t res = ldiv(n_tmp, 10);
            buf[i++] = '0' + abs((int)res.rem);
            n_tmp = res.quot;
        }
    }

    if (n < 0)
        buf[i++] = '-';

    size_t pad_size = i >= ctx->min_width ? 0 : ctx->min_width - i;

    while (pad_size-- > 0)
        vga_putchar(ctx->pad_char);

    while (i > 0)
        vga_putchar(buf[--i]);
}

static void print_lu(unsigned long n, const PrintCtx ctx[static const 1])
{
    static constexpr size_t BUF_SIZE = MAX_DEC_DIGITS(n);
    char buf[BUF_SIZE];

    size_t i = 0;

    if (n == 0) {
        buf[i++] = '0';
    } else {
        while (n != 0) {
            const uldiv_t res = uldiv(n, 10);
            buf[i++] = '0' + res.rem;
            n = res.quot;
        }
    }

    size_t pad_size = i >= ctx->min_width ? 0 : ctx->min_width - i;

    while (pad_size-- > 0)
        vga_putchar(ctx->pad_char);

    while (i > 0)
        vga_putchar(buf[--i]);
}

static void print_hex(const unsigned long n, const char a_base,
                      const PrintCtx ctx[static const 1])
{
    static constexpr size_t HEX_DIGITS = (CHAR_BIT / 4) * sizeof(n);

    unsigned i = 0;

    if (n == 0) {
        i = 1;
    } else {
        i = HEX_DIGITS;

        while (i > 0 && n >> (4 * (i - 1)) == 0)
            --i;
    }

    size_t pad_size = i >= ctx->min_width ? 0 : ctx->min_width - i;

    while (pad_size-- > 0)
        vga_putchar(ctx->pad_char);

    while (i > 0) {
        const unsigned nib = (n >> (4 * --i)) & 0xF;
        vga_putchar(nib < 10 ? '0' + nib : a_base + nib - 10);
    }
}

static void print_bin(const unsigned long n, const PrintCtx ctx[static const 1])
{
    static constexpr size_t BIN_DIGITS = CHAR_BIT * sizeof(n);

    unsigned i = 0;

    if (n == 0) {
        i = 1;
    } else {
        i = BIN_DIGITS;

        while (i > 0 && n >> (i - 1) == 0)
            --i;
    }

    size_t pad_size = i >= ctx->min_width ? 0 : ctx->min_width - i;

    while (pad_size-- > 0)
        vga_putchar(ctx->pad_char);

    while (i > 0) {
        const unsigned b = (n >> --i) & 1;
        vga_putchar('0' + b);
    }
}

static void print_str(const char s[static 1], const PrintCtx *const ctx)
{
    if (ctx == nullptr) {
        while (*s != '\0')
            vga_putchar(*s++);
    } else {
        unsigned n = ctx->precision;

        while (*s != '\0' && n-- > 0)
            vga_putchar(*s++);
    }
}

void vprintk(const char format[static restrict 1], va_list args)
{
    char ch = '\0';

    while ((ch = *format++) != '\0') {
        if (ch != '%') {
            vga_putchar(ch);
            continue;
        }

        PrintCtx ctx;
        print_ctx_init(&ctx);

        if (*format == '0') {
            ctx.pad_char = '0';
            ++format;
        }

        if (*format == '*' || is_digit(*format)) {
            int read_value = -1;

            if (*format == '*') {
                ++format;
                read_value = va_arg(args, int);
            } else {
                char *end = nullptr;
                read_value = strtol(format, &end, 10);
                format = end;
            }

            if (read_value >= 0)
                ctx.min_width = read_value;
        }

        if (*format == '.') {
            ++format;
            int read_value = -1;

            if (*format == '*') {
                ++format;
                read_value = va_arg(args, int);
            } else {
                char *end = nullptr;
                read_value = strtol(format, &end, 10);
                format = end;
            }

            if (read_value >= 0)
                ctx.precision = read_value;
        }

        // clang-format off
        switch (*format++) {
        case '%': vga_putchar('%');               break;
        case 'c': vga_putchar(va_arg(args, int)); break;
        case 'd':
        case 'i': print_l(va_arg(args, int), &ctx);             break;
        case 'u': print_lu(va_arg(args, unsigned), &ctx);       break;
        case 's': print_str(va_arg(args, const char *), &ctx);  break;
        case 'x': print_hex(va_arg(args, unsigned), 'a', &ctx); break;
        case 'X': print_hex(va_arg(args, unsigned), 'A', &ctx); break;
        case 'b': print_bin(va_arg(args, unsigned), &ctx);      break;
        case 'B': print_bin(va_arg(args, unsigned), &ctx);      break;
            // clang-format on

        case 'p':
            auto const ptr = va_arg(args, const void *);

            if (ptr == nullptr) {
                print_str("(nil)", nullptr);
            } else {
                print_str("0x", nullptr);
                print_hex((uintptr_t)ptr, 'a', &ctx);
            }
            break;

        case 'z':
            switch (*format++) {
                // clang-format off
            case 'd':
            case 'i': print_l(va_arg(args, ptrdiff_t), &ctx);     break;
            case 'u': print_lu(va_arg(args, size_t), &ctx);       break;
            case 'x': print_hex(va_arg(args, size_t), 'a', &ctx); break;
            case 'X': print_hex(va_arg(args, size_t), 'A', &ctx); break;
            case 'b': print_bin(va_arg(args, size_t), &ctx);      break;
            case 'B': print_bin(va_arg(args, size_t), &ctx);      break;
            default:                                              break;
                // clang-format on
            }
            break;

        case 'l':
            switch (*format++) {
                // clang-format off
            case 'd':
            case 'i': print_l(va_arg(args, long), &ctx);                 break;
            case 'u': print_lu(va_arg(args, unsigned long), &ctx);       break;
            case 'x': print_hex(va_arg(args, unsigned long), 'a', &ctx); break;
            case 'X': print_hex(va_arg(args, unsigned long), 'A', &ctx); break;
            case 'b': print_bin(va_arg(args, unsigned long), &ctx);      break;
            case 'B': print_bin(va_arg(args, unsigned long), &ctx);      break;
            default:                                                     break;
                // clang-format on
            }
            break;

        default:
            break;
        }
    }
}

void printk(const char format[static const restrict 1], ...)
{
    va_list args; // NOLINT
    va_start(args, format);
    vprintk(format, args);
    va_end(args);
}
