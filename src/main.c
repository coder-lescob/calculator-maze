#include <eadk.h>
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include "vec.h"
#include "raycaster.h"
#include "maze.h"


// Yes I actually know that much
#define PI 3.141592653589793f

const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Maze";
const uint32_t eadk_api_level  __attribute__((section(".rodata.eadk_api_level"))) = 0;

int main(void) {
    Maze maze = { .width = 5, .height = 5, .tiles = calloc(sizeof(uint8_t), 5 * 5)};
    Vec2 pos = {0.5, 0.5};

    float angle = 0;

    while (true) {
        eadk_keyboard_state_t keyboad = eadk_keyboard_scan();

        if (eadk_keyboard_key_down(keyboad, eadk_key_back)) {
            break;
        }

        eadk_display_push_rect_uniform(eadk_screen_rect, eadk_color_black);
        

        angle += PI * (eadk_keyboard_key_down(keyboad, eadk_key_right) - eadk_keyboard_key_down(keyboad, eadk_key_left)) * 0.016;
        float movement = (eadk_keyboard_key_down(keyboad, eadk_key_up) - eadk_keyboard_key_down(keyboad, eadk_key_down)) * 0.016;

        pos = vec_add(pos, vec_scale(movement, (Vec2){cosf(angle), sinf(angle)}));

        raycast_render(pos, &maze, angle);
        
        char msg[50] = {0};
        snprintf(msg, 49, "%f %f angle: %f", pos.x, pos.y, angle);
        eadk_display_draw_string(msg, (eadk_point_t) { 0, 0 }, false, eadk_color_white, eadk_color_black);

        // ~60 FPS
        eadk_timing_msleep(16);
    }
}