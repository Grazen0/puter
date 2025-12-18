#ifndef FIRMWARE_KEYBOARD_H
#define FIRMWARE_KEYBOARD_H

#include "numeric.h"
#include <stddef.h>

typedef enum : u8 {
    KEY_BACKTICK,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_0,
    KEY_DASH,
    KEY_EQUALS,
    KEY_BACKSPACE,
    KEY_TAB,
    KEY_Q,
    KEY_W,
    KEY_E,
    KEY_R,
    KEY_T,
    KEY_Y,
    KEY_U,
    KEY_I,
    KEY_O,
    KEY_P,
    KEY_LBRACKET,
    KEY_RBRACKET,
    KEY_CAPS_LOCK,
    KEY_A,
    KEY_S,
    KEY_D,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_SEMICOLON,
    KEY_QUOTE,
    KEY_ENTER,
    KEY_LSHIFT,
    KEY_Z,
    KEY_X,
    KEY_C,
    KEY_V,
    KEY_B,
    KEY_N,
    KEY_M,
    KEY_COMMA,
    KEY_PERIOD,
    KEY_SLASH,
    KEY_RSHIFT,
    KEY_LCTRL,
    KEY_LALT,
    KEY_SPACE,
    KEY_RALT,
    KEY_RCTRL,
    KEY_INSERT,
    KEY_DELETE,
    KEY_LEFT_ARROW,
    KEY_HOME,
    KEY_END,
    KEY_UP_ARROW,
    KEY_DOWN_ARROW,
    KEY_PAGE_UP,
    KEY_PAGE_DOWN,
    KEY_RIGHT_ARROW,
    KEY_NUM_LOCK,
    KEY_KEYPAD_7,
    KEY_KEYPAD_4,
    KEY_KEYPAD_1,
    KEY_KEYPAD_SLASH,
    KEY_KEYPAD_8,
    KEY_KEYPAD_5,
    KEY_KEYPAD_2,
    KEY_KEYPAD_0,
    KEY_KEYPAD_MUL,
    KEY_KEYPAD_9,
    KEY_KEYPAD_6,
    KEY_KEYPAD_3,
    KEY_KEYPAD_PERIOD,
    KEY_KEYPAD_DASH,
    KEY_KEYPAD_PLUS,
    KEY_KEYPAD_ENTER,
    KEY_ESC,
    KEY_F1,
    KEY_F2,
    KEY_F3,
    KEY_F4,
    KEY_F5,
    KEY_F6,
    KEY_F7,
    KEY_F8,
    KEY_F9,
    KEY_F10,
    KEY_F11,
    KEY_F12,
    KEY_SCROLL_LOCK,
    KEY_BACKSLASH,
} KeyCode;

typedef enum : u8 {
    KMOD_NONE = 0,
    KMOD_LCTRL = 1 << 0,
    KMOD_RCTRL = 1 << 1,
    KMOD_LSHIFT = 1 << 2,
    KMOD_RSHIFT = 1 << 3,
    KMOD_LALT = 1 << 4,
    KMOD_RALT = 1 << 5,

    KMOD_CTRL = KMOD_LCTRL | KMOD_RCTRL,
    KMOD_SHIFT = KMOD_LSHIFT | KMOD_RSHIFT,
    KMOD_ALT = KMOD_LALT | KMOD_RALT,
} KeyMod;

typedef struct {
    KeyCode code;
    KeyMod mod;
} Key;

void kb_init(void);

void kb_process_interrupt(void);

void kb_process_queue(void);

bool kb_poll_key(Key *out);

#endif
