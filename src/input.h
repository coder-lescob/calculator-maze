#ifndef INPUT_H
#define INPUT_H

#include "platform.h"

// TYPES WRAPPERS

// numworks platforms
#ifdef PLATFORM_CALCULATOR

#include <eadk.h>

typedef eadk_keyboard_state_t keyboard_t;

typedef enum {
    key_left = 0,
    key_up = 1,
    key_down = 2,
    key_right = 3,
    key_ok = 4,
    key_back = 5,
    key_home = 6,
    key_on_off = 8,
    key_shift = 12,
    key_alpha = 13,
    key_xnt = 14,
    key_var = 15,
    key_toolbox = 16,
    key_backspace = 17,
    key_exp = 18,
    key_ln = 19,
    key_log = 20,
    key_imaginary = 21,
    key_comma = 22,
    key_power = 23,
    key_sine = 24,
    key_cosine = 25,
    key_tangent = 26,
    key_pi = 27,
    key_sqrt = 28,
    key_square = 29,
    key_seven = 30,
    key_eight = 31,
    key_nine = 32,
    key_left_parenthesis = 33,
    key_right_parenthesis = 34,
    key_four = 36,
    key_five = 37,
    key_six = 38,
    key_multiplication = 39,
    key_division = 40,
    key_one = 42,
    key_two = 43,
    key_three = 44,
    key_plus = 45,
    key_minus = 46,
    key_zero = 48,
    key_dot = 49,
    key_ee = 50,
    key_ans = 51,
    key_exe = 52
  } keyboard_key_t;

#else
#error PLATFORM NOT SUPPORTED
#endif

// FUNCTION WRAPPERS

/**
 * create a new keyboard object
 */
keyboard_t create_keyboard();

/**
 * destroys the keyboard object
 */
void destroy_keyboard(keyboard_t *keyboard);

/**
 * polls the keyboard
 */
void keyboard_poll(keyboard_t *keyboard);

/**
 * get if a key is pressed
 */
bool key_pressed(keyboard_t *keyboard, keyboard_key_t key);

#endif