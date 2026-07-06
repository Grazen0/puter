#include "keyboard.h"
#include "control.h"
#include "puter.h"
#include <stdio.h>
#include <string.h>

typedef enum : u8 {
    ScanCode_Backtick = 0x0E,
    ScanCode_1 = 0x16,
    ScanCode_2 = 0x1E,
    ScanCode_3 = 0x26,
    ScanCode_4 = 0x25,
    ScanCode_5 = 0x2E,
    ScanCode_6 = 0x36,
    ScanCode_7 = 0x3D,
    ScanCode_8 = 0x3E,
    ScanCode_9 = 0x46,
    ScanCode_0 = 0x45,
    ScanCode_Dash = 0x4E,
    ScanCode_Equals = 0x55,
    ScanCode_Backspace = 0x66,
    ScanCode_Tab = 0x0D,
    ScanCode_Q = 0x15,
    ScanCode_W = 0x1D,
    ScanCode_E = 0x24,
    ScanCode_R = 0x2D,
    ScanCode_T = 0x2C,
    ScanCode_Y = 0x35,
    ScanCode_U = 0x3C,
    ScanCode_I = 0x43,
    ScanCode_O = 0x44,
    ScanCode_P = 0x4D,
    ScanCode_LeftBracket = 0x54,
    ScanCode_RightBracket = 0x5B,
    ScanCode_CapsLock = 0x58,
    ScanCode_A = 0x1C,
    ScanCode_S = 0x1B,
    ScanCode_D = 0x23,
    ScanCode_F = 0x2B,
    ScanCode_G = 0x34,
    ScanCode_H = 0x33,
    ScanCode_J = 0x3B,
    ScanCode_K = 0x42,
    ScanCode_L = 0x4B,
    ScanCode_Semicolon = 0x4C,
    ScanCode_Quote = 0x52,
    ScanCode_Enter = 0x5A,
    ScanCode_LeftShift = 0x12,
    ScanCode_Z = 0x1A,
    ScanCode_X = 0x22,
    ScanCode_C = 0x21,
    ScanCode_V = 0x2A,
    ScanCode_B = 0x32,
    ScanCode_N = 0x31,
    ScanCode_M = 0x3A,
    ScanCode_Comma = 0x41,
    ScanCode_Period = 0x49,
    ScanCode_Slash = 0x4A,
    ScanCode_RightShift = 0x59,
    ScanCode_LeftCtrl = 0x14,
    ScanCode_LeftAlt = 0x11,
    ScanCode_Space = 0x29,
    ScanCode_ExtRightAlt = 0x11,
    ScanCode_ExtRightCtrl = 0x14,
    ScanCode_ExtInsert = 0x70,
    ScanCode_ExtDelete = 0x71,
    ScanCode_ExtLeftArrow = 0x6B,
    ScanCode_ExtHome = 0x6C,
    ScanCode_ExtEnd = 0x69,
    ScanCode_ExtUpArrow = 0x75,
    ScanCode_ExtDownArrow = 0x72,
    ScanCode_ExtPageUp = 0x7D,
    ScanCode_ExtPageDown = 0x7A,
    ScanCode_ExtRightArrow = 0x74,
    ScanCode_NumLock = 0x77,
    ScanCode_Keypad7 = 0x6C,
    ScanCode_Keypad4 = 0x6B,
    ScanCode_Keypad1 = 0x69,
    ScanCode_ExtKeypadSlash = 0x4A,
    ScanCode_Keypad8 = 0x75,
    ScanCode_Keypad5 = 0x73,
    ScanCode_Keypad2 = 0x72,
    ScanCode_Keypad0 = 0x70,
    ScanCode_KeypadMul = 0x7C,
    ScanCode_Keypad9 = 0x7D,
    ScanCode_Keypad6 = 0x74,
    ScanCode_Keypad3 = 0x7A,
    ScanCode_KeypadPeriod = 0x71,
    ScanCode_KeypadDash = 0x7B,
    ScanCode_KeypadPlus = 0x79,
    ScanCode_ExtKeypadEnter = 0x5A,
    ScanCode_Esc = 0x76,
    ScanCode_F1 = 0x05,
    ScanCode_F2 = 0x06,
    ScanCode_F3 = 0x04,
    ScanCode_F4 = 0x0C,
    ScanCode_F5 = 0x03,
    ScanCode_F6 = 0x0B,
    ScanCode_F7 = 0x83,
    ScanCode_F8 = 0x0A,
    ScanCode_F9 = 0x01,
    ScanCode_F10 = 0x09,
    ScanCode_F11 = 0x78,
    ScanCode_F12 = 0x07,
    ScanCode_ScrollLock = 0x7E,
    ScanCode_Backslash = 0x5D,

    ScanCode_Break = 0xF0,
    ScanCode_Ext = 0xE0,
} ScanCode;

typedef enum {
    State_Idle,
    State_Break,
    State_Ext,
    State_ExtBreak,
} State;

static const KeyCode scancode_to_keycode[256] = {
    [ScanCode_Backtick] = KeyCode_Backtick,
    [ScanCode_1] = KeyCode_1,
    [ScanCode_2] = KeyCode_2,
    [ScanCode_3] = KeyCode_3,
    [ScanCode_4] = KeyCode_4,
    [ScanCode_5] = KeyCode_5,
    [ScanCode_6] = KeyCode_6,
    [ScanCode_7] = KeyCode_7,
    [ScanCode_8] = KeyCode_8,
    [ScanCode_9] = KeyCode_9,
    [ScanCode_0] = KeyCode_0,
    [ScanCode_Dash] = KeyCode_Dash,
    [ScanCode_Equals] = KeyCode_Equals,
    [ScanCode_Backspace] = KeyCode_Backspace,
    [ScanCode_Tab] = KeyCode_Tab,
    [ScanCode_Q] = KeyCode_Q,
    [ScanCode_W] = KeyCode_W,
    [ScanCode_E] = KeyCode_E,
    [ScanCode_R] = KeyCode_R,
    [ScanCode_T] = KeyCode_T,
    [ScanCode_Y] = KeyCode_Y,
    [ScanCode_U] = KeyCode_U,
    [ScanCode_I] = KeyCode_I,
    [ScanCode_O] = KeyCode_O,
    [ScanCode_P] = KeyCode_P,
    [ScanCode_LeftBracket] = KeyCode_LBracket,
    [ScanCode_RightBracket] = KeyCode_RBracket,
    [ScanCode_A] = KeyCode_A,
    [ScanCode_S] = KeyCode_S,
    [ScanCode_D] = KeyCode_D,
    [ScanCode_F] = KeyCode_F,
    [ScanCode_G] = KeyCode_G,
    [ScanCode_H] = KeyCode_H,
    [ScanCode_J] = KeyCode_J,
    [ScanCode_K] = KeyCode_K,
    [ScanCode_L] = KeyCode_L,
    [ScanCode_Semicolon] = KeyCode_Semicolon,
    [ScanCode_Quote] = KeyCode_Quote,
    [ScanCode_Enter] = KeyCode_Enter,
    [ScanCode_Z] = KeyCode_Z,
    [ScanCode_X] = KeyCode_X,
    [ScanCode_C] = KeyCode_C,
    [ScanCode_V] = KeyCode_V,
    [ScanCode_B] = KeyCode_B,
    [ScanCode_N] = KeyCode_N,
    [ScanCode_M] = KeyCode_M,
    [ScanCode_Comma] = KeyCode_Comma,
    [ScanCode_Period] = KeyCode_Period,
    [ScanCode_Slash] = KeyCode_Slash,
    [ScanCode_Space] = KeyCode_Space,
    [ScanCode_Keypad7] = KeyCode_Keypad7,
    [ScanCode_Keypad4] = KeyCode_Keypad4,
    [ScanCode_Keypad1] = KeyCode_Keypad1,
    [ScanCode_Keypad8] = KeyCode_Keypad8,
    [ScanCode_Keypad5] = KeyCode_Keypad5,
    [ScanCode_Keypad2] = KeyCode_Keypad2,
    [ScanCode_Keypad0] = KeyCode_Keypad0,
    [ScanCode_KeypadMul] = KeyCode_KeypadMul,
    [ScanCode_Keypad9] = KeyCode_Keypad9,
    [ScanCode_Keypad6] = KeyCode_Keypad6,
    [ScanCode_Keypad3] = KeyCode_Keypad3,
    [ScanCode_KeypadPeriod] = KeyCode_KeypadPeriod,
    [ScanCode_KeypadDash] = KeyCode_KeypadDash,
    [ScanCode_KeypadPlus] = KeyCode_KeypadPlus,
    [ScanCode_Esc] = KeyCode_Esc,
    [ScanCode_F1] = KeyCode_F1,
    [ScanCode_F2] = KeyCode_F2,
    [ScanCode_F3] = KeyCode_F3,
    [ScanCode_F4] = KeyCode_F4,
    [ScanCode_F5] = KeyCode_F5,
    [ScanCode_F6] = KeyCode_F6,
    [ScanCode_F7] = KeyCode_F7,
    [ScanCode_F8] = KeyCode_F8,
    [ScanCode_F9] = KeyCode_F9,
    [ScanCode_F10] = KeyCode_F10,
    [ScanCode_F11] = KeyCode_F11,
    [ScanCode_F12] = KeyCode_F12,
    [ScanCode_ScrollLock] = KeyCode_ScrollLock,
    [ScanCode_Backslash] = KeyCode_Backslash,
};

static const KeyCode scancode_ext_to_keycode[256] = {
    [ScanCode_ExtInsert] = KeyCode_Insert,
    [ScanCode_ExtDelete] = KeyCode_Delete,
    [ScanCode_ExtLeftArrow] = KeyCode_LeftArrow,
    [ScanCode_ExtHome] = KeyCode_Home,
    [ScanCode_ExtEnd] = KeyCode_End,
    [ScanCode_ExtUpArrow] = KeyCode_UpArrow,
    [ScanCode_ExtDownArrow] = KeyCode_DownArrow,
    [ScanCode_ExtPageUp] = KeyCode_PageUp,
    [ScanCode_ExtPageDown] = KeyCode_PageDown,
    [ScanCode_ExtRightArrow] = KeyCode_RightArrow,
    [ScanCode_ExtKeypadSlash] = KeyCode_KeypadSlash,
    [ScanCode_ExtKeypadEnter] = KeyCode_KeypadEnter,
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

static inline bool kb_scancode_available()
{
    return ctx.sc_head != ctx.sc_tail;
}

static u8 kb_scancode_take()
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

static KeyMod make_keymod()
{
    KeyMod mod = KeyMod_None;

    if (ctx.lshift)
        mod |= KeyMode_LShift;

    if (ctx.rshift)
        mod |= KeyMod_RShift;

    if (ctx.lalt)
        mod |= KeyMode_LShift;

    if (ctx.ralt)
        mod |= KeyMod_RShift;

    if (ctx.lctrl)
        mod |= KeyMode_LShift;

    if (ctx.rctrl)
        mod |= KeyMod_RShift;

    return mod;
}

static inline void kb_key_push(const Key key[static const 1])
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
    case State_Break:
    case State_ExtBreak:
        // Ignore key release
        ctx.state = State_Idle;
        break;

    case State_Idle: {
        if (scancode == ScanCode_Break) {
            ctx.state = State_Break;
        } else if (scancode == ScanCode_Ext) {
            ctx.state = State_Ext;
        } else {
            const KeyCode keycode = scancode_to_keycode[scancode];

            if (keycode != 0)
                kb_key_push_keycode(keycode);
        }
        break;
    }

    case State_Ext: {
        if (scancode == ScanCode_Break) {
            ctx.state = State_ExtBreak;
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

void kb_init()
{
    ctx = (KeyboardContext){
        .sc_head = 0,
        .sc_tail = 0,

        .kb_head = 0,
        .kb_tail = 0,

        .state = State_Idle,
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

    PLIC->int_enable[MeiId_Keyboard] = true;
}

void kb_process_int()
{
    kb_scancode_push(KEYBOARD->scancode);
}

void kb_process_queue()
{
    while (kb_scancode_available())
        kb_process_scancode(kb_scancode_take());
}

bool kb_poll_key(Key out[static const 1])
{
    if (ctx.kb_head == ctx.kb_tail)
        return false;

    *out = ctx.key_buf[ctx.kb_head++];

    if (ctx.kb_head >= KEYBUF_CAPACITY)
        ctx.kb_head = 0;

    return true;
}
