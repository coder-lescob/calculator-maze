#include "input.h"

// FUNCTION WRAPPERS

#ifdef PLATFORM_CALCULATOR

#include <eadk.h>

/**
 * create a new keyboard object
 */
keyboard_t create_keyboard() {
    // return the empty keyboard state
    return 0;
}

/**
 * destroys the keyboard object
 */
void destroy_keyboard(keyboard_t *keyboard) {
    // the compiler already destroys this integer so let him do so
    (void)keyboard; // ignore argument
}

/**
 * polls the keyboard
 */
void keyboard_poll(keyboard_t *keyboard) {
    *keyboard = eadk_keyboard_scan();
}

/**
 * get if a key is pressed
 */
bool key_pressed(keyboard_t *keyboard, key_t key) {
    return eadk_keyboard_key_down(
        *keyboard,
        key
    );
}

#else
#error PLATFORM NOT SUPPORTED
#endif