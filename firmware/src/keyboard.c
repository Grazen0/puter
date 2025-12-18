#include "keyboard.h"
#include "control.h"
#include "puter.h"
#include "vga.h"
#include <stdio.h>
#include <string.h>

typedef enum : u8 {
    SC_BACKTICK = 0x0E,
    SC_1 = 0x16,
    SC_2 = 0x1E,
    SC_3 = 0x26,
    SC_4 = 0x25,
    SC_5 = 0x2E,
    SC_6 = 0x36,
    SC_7 = 0x3D,
    SC_8 = 0x3E,
    SC_9 = 0x46,
    SC_0 = 0x45,
    SC_DASH = 0x4E,
    SC_EQUALS = 0x55,
    SC_BACKSPACE = 0x66,
    SC_TAB = 0x0D,
    SC_Q = 0x15,
    SC_W = 0x1D,
    SC_E = 0x24,
    SC_R = 0x2D,
    SC_T = 0x2C,
    SC_Y = 0x35,
    SC_U = 0x3C,
    SC_I = 0x43,
    SC_O = 0x44,
    SC_P = 0x4D,
    SC_LEFT_BRACKET = 0x54,
    SC_RIGHT_BRACKET = 0x5B,
    SC_CAPS_LOCK = 0x58,
    SC_A = 0x1C,
    SC_S = 0x1B,
    SC_D = 0x23,
    SC_F = 0x2B,
    SC_G = 0x34,
    SC_H = 0x33,
    SC_J = 0x3B,
    SC_K = 0x42,
    SC_L = 0x4B,
    SC_SEMICOLON = 0x4C,
    SC_QUOTE = 0x52,
    SC_ENTER = 0x5A,
    SC_LEFT_SHIFT = 0x12,
    SC_Z = 0x1A,
    SC_X = 0x22,
    SC_C = 0x21,
    SC_V = 0x2A,
    SC_B = 0x32,
    SC_N = 0x31,
    SC_M = 0x3A,
    SC_COMMA = 0x41,
    SC_PERIOD = 0x49,
    SC_SLASH = 0x4A,
    SC_RIGHT_SHIFT = 0x59,
    SC_LEFT_CTRL = 0x14,
    SC_LEFT_ALT = 0x11,
    SC_SPACE = 0x29,
    SC_EXT_RIGHT_ALT = 0x11,
    SC_EXT_RIGHT_CTRL = 0x14,
    SC_EXT_INSERT = 0x70,
    SC_EXT_DELETE = 0x71,
    SC_EXT_LEFT_ARROW = 0x6B,
    SC_EXT_HOME = 0x6C,
    SC_EXT_END = 0x69,
    SC_EXT_UP_ARROW = 0x75,
    SC_EXT_DOWN_ARROW = 0x72,
    SC_EXT_PAGE_UP = 0x7D,
    SC_EXT_PAGE_DOWN = 0x7A,
    SC_EXT_RIGHT_ARROW = 0x74,
    SC_NUM_LOCK = 0x77,
    SC_KEYPAD_7 = 0x6C,
    SC_KEYPAD_4 = 0x6B,
    SC_KEYPAD_1 = 0x69,
    SC_EXT_KEYPAD_SLASH = 0x4A,
    SC_KEYPAD_8 = 0x75,
    SC_KEYPAD_5 = 0x73,
    SC_KEYPAD_2 = 0x72,
    SC_KEYPAD_0 = 0x70,
    SC_KEYPAD_MUL = 0x7C,
    SC_KEYPAD_9 = 0x7D,
    SC_KEYPAD_6 = 0x74,
    SC_KEYPAD_3 = 0x7A,
    SC_KEYPAD_PERIOD = 0x71,
    SC_KEYPAD_DASH = 0x7B,
    SC_KEYPAD_PLUS = 0x79,
    SC_EXT_KEYPAD_ENTER = 0x5A,
    SC_ESC = 0x76,
    SC_F1 = 0x05,
    SC_F2 = 0x06,
    SC_F3 = 0x04,
    SC_F4 = 0x0C,
    SC_F5 = 0x03,
    SC_F6 = 0x0B,
    SC_F7 = 0x83,
    SC_F8 = 0x0A,
    SC_F9 = 0x01,
    SC_F10 = 0x09,
    SC_F11 = 0x78,
    SC_F12 = 0x07,
    SC_SCROLL_LOCK = 0x7E,
    SC_BACKSLASH = 0x5D,

    SC_BREAK = 0xF0,
    SC_EXT = 0xE0,
} KeyScanCode;

typedef enum {
    STATE_IDLE,
    STATE_BREAK,
    STATE_EXT,
    STATE_EXT_BREAK,
} State;

static const KeyCode scancode_to_keycode[256] = {
    [SC_BACKTICK] = KEY_BACKTICK,
    [SC_1] = KEY_1,
    [SC_2] = KEY_2,
    [SC_3] = KEY_3,
    [SC_4] = KEY_4,
    [SC_5] = KEY_5,
    [SC_6] = KEY_6,
    [SC_7] = KEY_7,
    [SC_8] = KEY_8,
    [SC_9] = KEY_9,
    [SC_0] = KEY_0,
    [SC_DASH] = KEY_DASH,
    [SC_EQUALS] = KEY_EQUALS,
    [SC_BACKSPACE] = KEY_BACKSPACE,
    [SC_TAB] = KEY_TAB,
    [SC_Q] = KEY_Q,
    [SC_W] = KEY_W,
    [SC_E] = KEY_E,
    [SC_R] = KEY_R,
    [SC_T] = KEY_T,
    [SC_Y] = KEY_Y,
    [SC_U] = KEY_U,
    [SC_I] = KEY_I,
    [SC_O] = KEY_O,
    [SC_P] = KEY_P,
    [SC_LEFT_BRACKET] = KEY_LBRACKET,
    [SC_RIGHT_BRACKET] = KEY_RBRACKET,
    [SC_CAPS_LOCK] = KEY_CAPS_LOCK,
    [SC_A] = KEY_A,
    [SC_S] = KEY_S,
    [SC_D] = KEY_D,
    [SC_F] = KEY_F,
    [SC_G] = KEY_G,
    [SC_H] = KEY_H,
    [SC_J] = KEY_J,
    [SC_K] = KEY_K,
    [SC_L] = KEY_L,
    [SC_SEMICOLON] = KEY_SEMICOLON,
    [SC_QUOTE] = KEY_QUOTE,
    [SC_ENTER] = KEY_ENTER,
    [SC_LEFT_SHIFT] = KEY_LSHIFT,
    [SC_Z] = KEY_Z,
    [SC_X] = KEY_X,
    [SC_C] = KEY_C,
    [SC_V] = KEY_V,
    [SC_B] = KEY_B,
    [SC_N] = KEY_N,
    [SC_M] = KEY_M,
    [SC_COMMA] = KEY_COMMA,
    [SC_PERIOD] = KEY_PERIOD,
    [SC_SLASH] = KEY_SLASH,
    [SC_RIGHT_SHIFT] = KEY_RSHIFT,
    [SC_LEFT_CTRL] = KEY_LCTRL,
    [SC_LEFT_ALT] = KEY_LALT,
    [SC_SPACE] = KEY_SPACE,
    [SC_NUM_LOCK] = KEY_NUM_LOCK,
    [SC_KEYPAD_7] = KEY_KEYPAD_7,
    [SC_KEYPAD_4] = KEY_KEYPAD_4,
    [SC_KEYPAD_1] = KEY_KEYPAD_1,
    [SC_KEYPAD_8] = KEY_KEYPAD_8,
    [SC_KEYPAD_5] = KEY_KEYPAD_5,
    [SC_KEYPAD_2] = KEY_KEYPAD_2,
    [SC_KEYPAD_0] = KEY_KEYPAD_0,
    [SC_KEYPAD_MUL] = KEY_KEYPAD_MUL,
    [SC_KEYPAD_9] = KEY_KEYPAD_9,
    [SC_KEYPAD_6] = KEY_KEYPAD_6,
    [SC_KEYPAD_3] = KEY_KEYPAD_3,
    [SC_KEYPAD_PERIOD] = KEY_KEYPAD_PERIOD,
    [SC_KEYPAD_DASH] = KEY_KEYPAD_DASH,
    [SC_KEYPAD_PLUS] = KEY_KEYPAD_PLUS,
    [SC_ESC] = KEY_ESC,
    [SC_F1] = KEY_F1,
    [SC_F2] = KEY_F2,
    [SC_F3] = KEY_F3,
    [SC_F4] = KEY_F4,
    [SC_F5] = KEY_F5,
    [SC_F6] = KEY_F6,
    [SC_F7] = KEY_F7,
    [SC_F8] = KEY_F8,
    [SC_F9] = KEY_F9,
    [SC_F10] = KEY_F10,
    [SC_F11] = KEY_F11,
    [SC_F12] = KEY_F12,
    [SC_SCROLL_LOCK] = KEY_SCROLL_LOCK,
    [SC_BACKSLASH] = KEY_BACKSLASH,
};

static const KeyCode scancode_ext_to_keycode[256] = {
    [SC_EXT_RIGHT_ALT] = KEY_RALT,
    [SC_EXT_RIGHT_CTRL] = KEY_RCTRL,
    [SC_EXT_INSERT] = KEY_INSERT,
    [SC_EXT_DELETE] = KEY_DELETE,
    [SC_EXT_LEFT_ARROW] = KEY_LEFT_ARROW,
    [SC_EXT_HOME] = KEY_HOME,
    [SC_EXT_END] = KEY_END,
    [SC_EXT_UP_ARROW] = KEY_UP_ARROW,
    [SC_EXT_DOWN_ARROW] = KEY_DOWN_ARROW,
    [SC_EXT_PAGE_UP] = KEY_PAGE_UP,
    [SC_EXT_PAGE_DOWN] = KEY_PAGE_DOWN,
    [SC_EXT_RIGHT_ARROW] = KEY_RIGHT_ARROW,
    [SC_EXT_KEYPAD_SLASH] = KEY_KEYPAD_SLASH,
    [SC_EXT_KEYPAD_ENTER] = KEY_KEYPAD_ENTER,
};

static constexpr size_t SCANCODES_CAPACITY = 32;
static constexpr size_t KEYBUF_CAPACITY = 64;

typedef struct {
    u8 scancodes[SCANCODES_CAPACITY];
    size_t sc_head;
    volatile size_t sc_tail;

    Key key_buf[KEYBUF_CAPACITY];
    size_t kb_head;
    size_t kb_tail;

    State state;
    bool lctrl;
    bool rctrl;
    bool lshift;
    bool rshift;
    bool lalt;
    bool ralt;
    bool caps_lock;
    bool num_lock;
    bool scroll_lock;

} KeyboardContext;

static KeyboardContext ctx;

static inline bool kb_scancode_available(void)
{
    return ctx.sc_head != ctx.sc_tail;
}

static u8 kb_scancode_take(void)
{
    const u8 scancode = ctx.scancodes[ctx.sc_head];

    if (++ctx.sc_head >= SCANCODES_CAPACITY)
        ctx.sc_head = 0;

    return scancode;
}

static inline void kb_scancode_push(const u8 scancode)
{
    ctx.scancodes[ctx.sc_tail] = scancode;

    if (++ctx.sc_tail >= SCANCODES_CAPACITY)
        ctx.sc_tail = 0;
}

static KeyMod make_keymod(void)
{
    KeyMod mod = KMOD_NONE;

    if (ctx.lshift)
        mod |= KMOD_LSHIFT;

    if (ctx.rshift)
        mod |= KMOD_RSHIFT;

    if (ctx.lalt)
        mod |= KMOD_LSHIFT;

    if (ctx.ralt)
        mod |= KMOD_RSHIFT;

    if (ctx.lctrl)
        mod |= KMOD_LSHIFT;

    if (ctx.rctrl)
        mod |= KMOD_RSHIFT;

    return mod;
}

static inline void kb_key_push(const Key *const key)
{
    memcpy(&ctx.key_buf[ctx.kb_tail], key, sizeof(*key));

    if (++ctx.kb_tail >= KEYBUF_CAPACITY)
        ctx.kb_tail = 0;
}

static inline void kb_key_push_keycode(const KeyCode keycode)
{
    const Key key = {
        .code = keycode,
        .mod = make_keymod(),
    };

    kb_key_push(&key);
}

static inline void kb_process_scancode(const u8 scancode)
{
    switch (ctx.state) {
    case STATE_BREAK:
    case STATE_EXT_BREAK:
        // Ignore key release
        ctx.state = STATE_IDLE;
        break;

    case STATE_IDLE: {
        if (scancode == SC_BREAK) {
            ctx.state = STATE_BREAK;
        } else if (scancode == SC_EXT) {
            ctx.state = STATE_EXT;
        } else {
            const KeyCode keycode = scancode_to_keycode[scancode];

            if (keycode != 0)
                kb_key_push_keycode(keycode);
        }
        break;
    }

    case STATE_EXT: {
        if (scancode == SC_BREAK) {
            ctx.state = STATE_EXT_BREAK;
        } else {
            const KeyCode keycode = scancode_ext_to_keycode[scancode];

            if (keycode != 0)
                kb_key_push_keycode(keycode);
        }
        break;
    }

    default:
        PANIC("unreachable");
    }
}

void kb_init(void)
{
    ctx = (KeyboardContext){
        .sc_head = 0,
        .sc_tail = 0,

        .kb_head = 0,
        .kb_tail = 0,

        .state = STATE_IDLE,
        .lctrl = false,
        .rctrl = false,
        .lshift = false,
        .rshift = false,
        .lalt = false,
        .ralt = false,
        .caps_lock = false,
        .num_lock = false,
        .scroll_lock = false,
    };
}

void kb_process_interrupt(void)
{
    kb_scancode_push(KEYBOARD->scancode);
}

void kb_process_queue(void)
{
    while (kb_scancode_available())
        kb_process_scancode(kb_scancode_take());
}

bool kb_poll_key(Key *const out)
{
    if (ctx.kb_head == ctx.kb_tail)
        return false;

    memcpy(out, &ctx.key_buf[ctx.kb_head], sizeof(*out));

    if (++ctx.kb_head >= KEYBUF_CAPACITY)
        ctx.kb_head = 0;

    return true;
}
