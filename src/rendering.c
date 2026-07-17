#include "rendering.h"

// FUNCTION WRAPPERS

// NUMWORKS CALCULATOR PLATFORM
#ifdef PLATFORM_CALCULATOR

// numworks api
#include <eadk.h>

/**
 * creates a renderer
 * @note MUST init SCREEN_WIDTH and SCREEN_HEIGHT to some value if there undefined for this platform
 */
void create_renderer() { /* the numworks has no rendering objects */ }

/**
 * destroys the renderer
 */
void destroy_renderer() { /* the numworks has no rendering objects */ }

/**
 * draws an uniform rectangle with the color color.
 */
void draw_rect_uniform(rect_t rect, color_t color) {
    eadk_display_push_rect_uniform(
        rect,
        color
    );
}

/**
 * draws a textured rectangle to the screen.
 * @note pixels MUST be of length (rect.width * rect.height)
 *  not doing so would result in segmentation fault
 */
void draw_rect_textured(rect_t rect, const color_t *pixels) {
    eadk_display_push_rect(
        rect,
        pixels
    );
}

/**
 * draw a white string on a black background using the little font (on the calculator)
 * @note str MUST be null terminated
 */
void draw_string(point_t point, const char *str) {
    eadk_display_draw_string(
        str, 
        point, 
        false, 
        eadk_color_white, 
        eadk_color_black
    );
}

/**
 * swap the backbuffer with the screen buffer
 * @note on the calculator it only waits for vblank
 */
void swap_buffer() {
    eadk_display_wait_for_vblank();
}

#else
#error PLATFORM NOT SUPPORTED
#endif