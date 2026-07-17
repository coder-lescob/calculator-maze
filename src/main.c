// stdlib
#include <stdio.h>
#include <math.h>
#include <malloc.h>

// system specific wrappers
#include "platform.h"
#include "rendering.h"
#include "input.h"
#include "timings.h"

// game files
#include "vec.h"
#include "raycaster.h"
#include "maze.h"
#include "player.h"
#include "entity.h"


#define VERSION "0.4.6"

#ifdef PLATFORM_CALCULATOR

// calculator specific sections
const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "DEAD END " VERSION;
const uint32_t eadk_api_level  __attribute__((section(".rodata.eadk_api_level"))) = 0;

#endif

int main(void) {
    create_renderer();
    eadk_keyboard_state_t keyboard = create_keyboard();

    Maze maze = generate_maze(20, 20);
    Player player = { .pos = (Vec2) { 0.5f, 0.5f }, .angle = 0 };

    uint64_t last_time = get_time();
    float dt = 0;

    Entity entities[] = {
        (Entity) { .entity_type = 1, .pos = (Vec2) { 0.5f, 1.5f }},
        (Entity) { .entity_type = 1, .pos = (Vec2) { 1.5f, 0.5f }},
        (Entity) { .entity_type = 0, .pos = (Vec2) { 1.5f, 1.5f }},
    };

    while (true) {
        keyboard_poll(&keyboard);

        if (key_pressed(&keyboard, key_back)) {
            break;
        }

        // move the player
        player_move(&player, keyboard, dt, &maze);

        // render the terain
        raycast_render(player, &maze, entities, sizeof(entities) / sizeof(Entity));
        
        // render fps
        char msg[50] = {0};
        snprintf(msg, 49, "FPS: %d", (uint16_t)((dt > 0)? 1 / dt : 100000.0f));
        draw_string( (eadk_point_t) { 0, 0 }, msg);

        swap_buffer();

        // compute delta time
        dt = (get_time() - last_time) / 1000.0f;
        last_time = get_time();
    }

    // destroy the keyboard
    destroy_keyboard(&keyboard);
    destroy_renderer();

    // free the maze
    free_maze(&maze);
}