#include <eadk.h>

const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Maze";
const uint32_t eadk_api_level  __attribute__((section(".rodata.eadk_api_level"))) = 0;

int main(void) {
    eadk_display_draw_string("Hello, world!", (eadk_point_t){0, 0}, true, eadk_color_black, eadk_color_white);
    
    while (true) {
        eadk_keyboard_state_t keyboad = eadk_keyboard_scan();

        if (eadk_keyboard_key_down(keyboad, eadk_key_back)) {
            break;
        }

        // ~60 FPS
        eadk_timing_msleep(16);
    }
}