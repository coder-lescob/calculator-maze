#include <eadk.h>
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include "vec.h"
#include "raycaster.h"
#include "maze.h"
#include "player.h"

#define VERSION ("0.2.0")

const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "Maze";
const uint32_t eadk_api_level  __attribute__((section(".rodata.eadk_api_level"))) = 0;

int main(void) {
    Maze maze = generate_maze(20, 20);
    maze.tiles[3] = 1;
    
    Player player = { .pos = (Vec2) { 0.5f, 0.5f }, .angle = 0 };

    uint64_t last_time = eadk_timing_millis();
    float dt = 0;

    while (true) {
        eadk_keyboard_state_t keyboard = eadk_keyboard_scan();

        if (eadk_keyboard_key_down(keyboard, eadk_key_back)) {
            break;
        }

        // move the player
        player_move(&player, keyboard, dt, &maze);

        // render the terain
        raycast_render(player.pos, &maze, player.angle);
        
        // render fps
        char msg[50] = {0};
        snprintf(msg, 49, "FPS: %d", (uint16_t)((dt > 0)? 1 / dt : 100000.0f));
        eadk_display_draw_string(msg, (eadk_point_t) { 0, 0 }, false, eadk_color_white, eadk_color_black);

        // make display way less buggy on the calculator
        eadk_display_wait_for_vblank();

        // compute delta time
        dt = (eadk_timing_millis() - last_time) / 1000.0f;
        last_time = eadk_timing_millis();
    }

    // free the maze
    free_maze(&maze);
}