#include "errors.h"
#include "platform.h"

#include "timings.h"
#include "unwind.h"
#include <stdlib.h>

#ifdef PLATFORM_CALCULATOR

#define QUIT_ERR_MSG "press [ok] to quit app"

#else
#error PLATFROM NOT SUPPORTED
#endif

/**
 * shows an error screen and waits until the ok key is pressed
 * it also formats the text so that it fits on the screen
 * and shows some help for the user
 */
void error_screen(char *msg) {

    // fill the screen with red
    draw_rect_uniform(eadk_screen_rect, eadk_color_red);

    /**
     * TODO: format msg text
     */
    draw_string(
        (point_t) { 0, 0 },
        msg
    );

    // display help message
    draw_string(
        (point_t) { 0, SCREEN_HEIGHT - LITTE_FONT_HEIGHT }, 
        QUIT_ERR_MSG
    );

    // create the keyboard
    keyboard_t keyboard = create_keyboard();

    // wait for user to release ok
    do {
        keyboard_poll(&keyboard);
    }
    while (key_pressed(&keyboard, key_ok));

    // then wait until he repressed it
    do {
        keyboard_poll(&keyboard);
    }
    while (!key_pressed(&keyboard, key_ok));

    // wait for user to release ok
    do {
        keyboard_poll(&keyboard);
    }
    while (key_pressed(&keyboard, key_ok));
    msleep(500);

    // destroy the keyboard
    destroy_keyboard(&keyboard);

    PANIC();
}