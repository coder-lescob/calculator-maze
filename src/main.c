// stdlib
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <stdbool.h>

// system specific wrappers
#include "platform.h"
#include "rendering.h"
#include "input.h"
#include "timings.h"
#include "texture_loader.h"

// game files
#include "vec.h"
#include "raycaster.h"
#include "maze.h"
#include "player.h"
#include "entity.h"
#include "fixed_points.h"

#define VERSION "0.5.0"

// Yes I actually know that much
#define PI 3.141592653589793f

#ifdef PLATFORM_CALCULATOR

// calculator specific sections
const char eadk_app_name[] __attribute__((section(".rodata.eadk_app_name"))) = "DEAD END " VERSION;
const uint32_t eadk_api_level  __attribute__((section(".rodata.eadk_api_level"))) = 0;

#endif

int main(void) {
    create_renderer();
    
    keyboard_t keyboard = create_keyboard();
    textures_t textures = load_textures();
    if (!textures.successful_load) {
        // exit application
        goto endapp_init;
    }
    
    uint64_t last_time = get_time();
    float dt = 0;

    draw_rect_textured(eadk_screen_rect, textures.title_screen);
    keyboard_poll(&keyboard);
    while (key_pressed(&keyboard, key_ok)) {
        keyboard_poll(&keyboard);

        // compute delta time
        dt = (get_time() - last_time) / 1000.0f;
        last_time = get_time();
    }
    while (!key_pressed(&keyboard, key_ok)) {
        keyboard_poll(&keyboard);

        // compute delta time
        dt = (get_time() - last_time) / 1000.0f;
        last_time = get_time();
    }

    // generate the maze and create the player object
    Maze maze = generate_maze(20, 20);
    Player player = new_player((Vec2) { 0.5f, 0.5f }, (Vec2) { 1, 0 }, PI/3 /* 60° */);

    // spawn the entities
    Entity entities[] = {
        (Entity) { .entity_type = 1, .pos = (Vec2) { 0.5f, 1.5f }},
        (Entity) { .entity_type = 1, .pos = (Vec2) { 1.5f, 0.5f }},
        (Entity) { .entity_type = 0, .pos = (Vec2) { 1.5f, 1.5f }},
    };

    // game loop
    while (true) {
        keyboard_poll(&keyboard);

        if (key_pressed(&keyboard, key_back)) {
            break;
        }

        // move the player
        player_move(&player, &keyboard, dt, &maze);

        // move entities
        for (uint16_t i = 0; i < sizeof(entities) / sizeof(Entity); i++) {
            entity_move(&entities[i], dt, &player);
        }

        LayeredTextures layers = {
            .num_rects = 0,
            .rects = NULL,
            .pixels_by_rect = NULL,
            .on_the_side = NULL,
            .layers      = NULL,
        };

        // render the terain
        raycast_render(player, &maze, entities, sizeof(entities) / sizeof(Entity), textures, layers);
        
        // render fps
        char msg[50] = {0};
        snprintf(msg, 49, "FPS: %d", (uint16_t)((dt > 0)? 1 / dt : 100000.0f));
        draw_string((point_t) { 0, 0 }, msg);

        swap_buffer();

        // compute delta time
        dt = (get_time() - last_time) / 1000.0f;
        last_time = get_time();
    }

    // free the maze
    free_maze(&maze);
    unload_textures(&textures);

endapp_init:
    // destroy the keyboard
    destroy_keyboard(&keyboard);
    destroy_renderer();
}
