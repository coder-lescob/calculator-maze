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

float raycast_for_distance(Ray ray, Maze *maze) {
    uint16_t tile_idx = get_tile_idx(ray.origine, maze);
    float distance = 0.0f;

    float ds = 0.01f;
    Vec2 du = vec_scale(ds, ray.direction);

    while (maze->tiles[tile_idx] == 0) {
        ray.origine = vec_add(ray.origine, du);

        if (ray.origine.x < 0 || ray.origine.x >= maze->width || 
            ray.origine.y < 0 || ray.origine.y >= maze->height) {
            break;
        }
        
        tile_idx = get_tile_idx(ray.origine, maze);
        distance += ds;
    }

    return distance;
}

void raycast_render(Vec2 pos, Maze *maze, float player_angle) {
    for (int i = 0; i <= EADK_SCREEN_WIDTH / 2; i++) {
        float t = (float)i / EADK_SCREEN_WIDTH * 2;
        float angle = (player_angle - PI / 4) + t * (PI / 2);

        // garentee to have length 1
        Vec2 dir = (Vec2) { cosf(angle), sinf(angle) };
        float distance = raycast_for_distance((Ray){pos, dir}, maze) * cosf(player_angle - angle);

        uint16_t wall_height = (uint16_t)(200.0 / distance);

        if (wall_height >= EADK_SCREEN_HEIGHT) {
            wall_height = EADK_SCREEN_HEIGHT;
        }

        eadk_display_push_rect_uniform((eadk_rect_t) { .x = 2 * i, .y = (EADK_SCREEN_HEIGHT - wall_height) / 2 , .width = 2, .height = wall_height},  eadk_color_blue);
    }
}