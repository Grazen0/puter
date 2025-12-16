#include "keyboard.h"

static constexpr size_t SCANCODES_CAPACITY = 16;

typedef struct {
    u8 scancodes[SCANCODES_CAPACITY];
    size_t head;
    volatile size_t tail;
} KeyboardContext;

static KeyboardContext ctx;

void kb_init(void)
{
    ctx = (KeyboardContext){
        .head = 0,
        .tail = 0,
    };
}

bool kb_scancode_available(void)
{
    return ctx.head != ctx.tail;
}

u8 kb_scancode_take(void)
{
    const u8 scancode = ctx.scancodes[ctx.head];

    if (++ctx.head >= SCANCODES_CAPACITY)
        ctx.head = 0;

    return scancode;
}

void kb_scancode_push(const u8 scancode)
{
    ctx.scancodes[ctx.tail] = scancode;

    if (++ctx.tail >= SCANCODES_CAPACITY)
        ctx.tail = 0;
}
