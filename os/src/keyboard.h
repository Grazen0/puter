#ifndef FIRMWARE_KEYBOARD_H
#define FIRMWARE_KEYBOARD_H

#include "numeric.h"
#include <stddef.h>

typedef enum : u8 {
    KeyCode_Backtick,
    KeyCode_1,
    KeyCode_2,
    KeyCode_3,
    KeyCode_4,
    KeyCode_5,
    KeyCode_6,
    KeyCode_7,
    KeyCode_8,
    KeyCode_9,
    KeyCode_0,
    KeyCode_Dash,
    KeyCode_Equals,
    KeyCode_Backspace,
    KeyCode_Tab,
    KeyCode_A,
    KeyCode_B,
    KeyCode_C,
    KeyCode_D,
    KeyCode_E,
    KeyCode_F,
    KeyCode_G,
    KeyCode_H,
    KeyCode_I,
    KeyCode_J,
    KeyCode_K,
    KeyCode_L,
    KeyCode_M,
    KeyCode_N,
    KeyCode_O,
    KeyCode_P,
    KeyCode_Q,
    KeyCode_R,
    KeyCode_S,
    KeyCode_T,
    KeyCode_U,
    KeyCode_V,
    KeyCode_W,
    KeyCode_X,
    KeyCode_Y,
    KeyCode_Z,
    KeyCode_LBracket,
    KeyCode_RBracket,
    KeyCode_Semicolon,
    KeyCode_Quote,
    KeyCode_Enter,
    KeyCode_Comma,
    KeyCode_Period,
    KeyCode_Slash,
    KeyCode_Space,
    KeyCode_Insert,
    KeyCode_Delete,
    KeyCode_LeftArrow,
    KeyCode_Home,
    KeyCode_End,
    KeyCode_UpArrow,
    KeyCode_DownArrow,
    KeyCode_PageUp,
    KeyCode_PageDown,
    KeyCode_RightArrow,
    KeyCode_Keypad7,
    KeyCode_Keypad4,
    KeyCode_Keypad1,
    KeyCode_KeypadSlash,
    KeyCode_Keypad8,
    KeyCode_Keypad5,
    KeyCode_Keypad2,
    KeyCode_Keypad0,
    KeyCode_KeypadMul,
    KeyCode_Keypad9,
    KeyCode_Keypad6,
    KeyCode_Keypad3,
    KeyCode_KeypadPeriod,
    KeyCode_KeypadDash,
    KeyCode_KeypadPlus,
    KeyCode_KeypadEnter,
    KeyCode_Esc,
    KeyCode_F1,
    KeyCode_F2,
    KeyCode_F3,
    KeyCode_F4,
    KeyCode_F5,
    KeyCode_F6,
    KeyCode_F7,
    KeyCode_F8,
    KeyCode_F9,
    KeyCode_F10,
    KeyCode_F11,
    KeyCode_F12,
    KeyCode_ScrollLock,
    KeyCode_Backslash,
} KeyCode;

typedef enum : u8 {
    KeyMod_None = 0,
    KeyMode_LCtrl = 1 << 0,
    KeyMode_RCtrl = 1 << 1,
    KeyMode_LShift = 1 << 2,
    KeyMod_RShift = 1 << 3,
    KeyMod_LAlt = 1 << 4,
    KeyMod_RAlt = 1 << 5,

    KeyMod_Ctrl = KeyMode_LCtrl | KeyMode_RCtrl,
    KeyMod_Shift = KeyMode_LShift | KeyMod_RShift,
    KeyMod_Alt = KeyMod_LAlt | KeyMod_RAlt,
} KeyMod;

typedef struct {
    KeyCode code;
    KeyMod mod;
} Key;

void kb_init();

void kb_process_int();

void kb_process_queue();

bool kb_poll_key(Key out[static 1]);

#endif
