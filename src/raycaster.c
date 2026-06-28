#include "raycaster.h"
#include "maze.h"
#include "assert.h"
#include "math.h"

#include <stdio.h>

// Yes I actually know that much
#define PI 3.141592653589793f

inline uint16_t get_tile_idx(Vec2 pos, Maze *maze) {
    assert(pos.x >= 0 && pos.x < maze->width);
    assert(pos.y >= 0 && pos.y < maze->height);
    return (uint16_t)pos.x + (uint16_t)pos.y * maze->height;
}

float raycast_for_distance(Vec2 pos, Vec2 dir, Maze *maze) {
    uint16_t tile_idx = get_tile_idx(pos, maze);
    float distance = 0.0f;

    float ds = 0.01f;
    Vec2 du = vec_scale(ds, dir);

    while (maze->tiles[tile_idx] == 0) {
        pos = vec_add(pos, du);

        if (pos.x < 0 || pos.x >= maze->width || 
            pos.y < 0 || pos.y >= maze->height) {
            break;
        }
        
        tile_idx = get_tile_idx(pos, maze);
        distance += ds;
    }

    return distance;
}

/**
 * @note the length of distance MUST be equal SCREEN_WIDTH
 */
void raycast_render(Vec2 pos, Maze *maze, float player_angle) {
    for (int i = 0; i <= EADK_SCREEN_WIDTH; i++) {
        float t = (float)i / EADK_SCREEN_WIDTH;
        float angle = (player_angle - PI / 4) + t * (PI / 2);

        // garentee to have length 1
        Vec2 dir = (Vec2) { cosf(angle), sinf(angle) };
        float distance = raycast_for_distance(pos, dir, maze);

        uint16_t wall_height = (uint16_t)(250.0 / distance);

        if (wall_height >= EADK_SCREEN_HEIGHT) {
            wall_height = EADK_SCREEN_HEIGHT;
        }

        eadk_display_push_rect_uniform((eadk_rect_t) { .x = i, .y = (EADK_SCREEN_HEIGHT - wall_height) / 2 , .width = 1, .height = wall_height},  eadk_color_blue);
    }
}