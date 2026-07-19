#ifndef RENDERING_H
#define RENDERING_H

#include "platform.h"

// TYPES
#ifdef PLATFORM_CALCULATOR

#include <eadk.h>

// constants
static const uint16_t SCREEN_WIDTH  = EADK_SCREEN_WIDTH;
static const uint16_t SCREEN_HEIGHT = EADK_SCREEN_HEIGHT;
static const uint16_t LITTE_FONT_HEIGHT = 14; // 14px

// other types
typedef eadk_point_t point_t;
typedef eadk_rect_t rect_t;
typedef eadk_color_t color_t;

#else
#error PLATFORM NOT SUPPORTED
#endif

// FUNCTION WRAPPERS

/**
 * creates a renderer
 * @note MUST init SCREEN_WIDTH and SCREEN_HEIGHT to some value if there undefined for this platform
 */
void create_renderer();

/**
 * destroys the renderer
 */
void destroy_renderer();

/**
 * draws an uniform rectangle with the color color.
 */
void draw_rect_uniform(rect_t rect, color_t color);

/**
 * draws a textured rectangle to the screen.
 * @note pixels MUST be of length rect.width * rect.height 
 *  not doing so would result in segmentation fault
 */
void draw_rect_textured(rect_t rect, const color_t *pixels);

/**
 * draw a white string on a black background using the little font (on the calculator)
 * @note str MUST be null terminated
 */
void draw_string(point_t point, const char *str);

/**
 * swap the backbuffer with the screen buffer
 * @note on the calculator it only waits for vblank
 */
void swap_buffer();

#endif