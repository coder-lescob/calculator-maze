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

HitInfo raycast_single_ray(Ray ray, Maze *maze) {
    #define FLOOR_u16(F) (uint16_t)F
    eadk_point_t map_pos = { FLOOR_u16(ray.origine.x), FLOOR_u16(ray.origine.y) };

    int8_t step_x, step_y;
    float side_distance_x, side_distance_y;

    if (ray.direction.x < 0) {

        // ray.direction.x < 0 so |ray.direction.x| = -ray.direction.x
        step_x = -1;
        side_distance_x = -(ray.origine.x - map_pos.x) / ray.direction.x;
    }
    else if (ray.direction.x == 0) {
        step_x = 0;
        side_distance_x = INFINITY;
    }
    else {
        step_x = 1;
        side_distance_x = (map_pos.x + 1 - ray.origine.x) / ray.direction.x;
    }


    if (ray.direction.y < 0) {

        // ray.direction.y < 0 so |ray.direction.y| = -ray.direction.y
        step_y = -1;
        side_distance_y = -(ray.origine.y - map_pos.y) / ray.direction.y;
    }
    else if (ray.direction.y == 0) {
        step_y = 0;
        side_distance_y = INFINITY;
    }
    else {
        step_y = 1;
        side_distance_y = (map_pos.y + 1 - ray.origine.y) / ray.direction.y;
    }


    float delta_distance_x, delta_distance_y;

    // int comparaison are faster
    if (step_x == 0) {
        delta_distance_x = INFINITY;
    }
    else {
        // if dir.x < 0 then step_x == -1 so step_x * ray.direction.x = |ray.direction.x|
        delta_distance_x = 1 / (step_x * ray.direction.x);
    }

    // int comparaison are faster
    if (step_y == 0) {
        delta_distance_y = INFINITY;
    }
    else {
        // if dir.y < 0 then step_y == -1 so step_y * ray.direction.y = |ray.direction.y|
        delta_distance_y = 1 / (step_y * ray.direction.y);
    }

    float distance = 0.0f;
    Side hit_side  = UNDEFINED_SIDE;
    uint16_t tile_idx;

    while (true) {
        if (side_distance_x < side_distance_y) {
            distance         = side_distance_x;
            side_distance_x += delta_distance_x;
            map_pos.x       += step_x;
            hit_side         = VERTICAL;
        }
        else {
            distance         = side_distance_y;
            side_distance_y += delta_distance_y;
            map_pos.y       += step_y;
            hit_side         = HORIZONTAL;
        }

        if (map_pos.x >= maze->width || map_pos.y >= maze->height) {
            return (HitInfo) {
                .distance = distance,
                .block_type = 0,
                .side = UNDEFINED_SIDE,
            };
        }

        // compute the tile index
        tile_idx = map_pos.x + map_pos.y * maze->width;

        if (maze->tiles[tile_idx] > 0) {
            break;
        }
    }

    uint16_t block_type = maze->tiles[tile_idx];

    return (HitInfo) {
        .distance = distance, 
        .block_type = block_type, 
        .side = hit_side
    };
}

void raycast_render(Vec2 pos, Maze *maze, float player_angle) {
    size_t res = 1;

    for (int i = 0; i <= EADK_SCREEN_WIDTH / res; i++) {
        float t = (float)i / EADK_SCREEN_WIDTH * res;
        float angle = (player_angle - PI / 4) + t * (PI / 2);

        // garentee to have length 1 then apply correction on distance to avoid fish eye len effect
        Vec2 dir = (Vec2) { cosf(angle), sinf(angle) };
        HitInfo hitInfo = raycast_single_ray((Ray) { pos, dir }, maze);
        float distance = hitInfo.distance * cosf(player_angle - angle);

        uint16_t wall_height = (uint16_t)(200.0 / distance);

        if (wall_height >= EADK_SCREEN_HEIGHT) {
            wall_height = EADK_SCREEN_HEIGHT;
        }

        eadk_color_t color;
        switch (hitInfo.block_type) {
            case 1:
                color = (hitInfo.side == HORIZONTAL)? eadk_color_blue : eadk_color_green;
                break;
            
            default:
                color = eadk_color_red;
                break;
        }

        eadk_display_push_rect_uniform(
            (eadk_rect_t) { 
                .x = res * i, 
                .y = (EADK_SCREEN_HEIGHT - wall_height) / 2 , 
                .width = res, 
                .height = wall_height,
            }, 
            color
        );

        eadk_display_push_rect_uniform(
            (eadk_rect_t) {
                .x = res * i,
                .y = 0,
                .width = res,
                .height = (EADK_SCREEN_HEIGHT - wall_height) / 2,
            },
            eadk_color_black
        );

        eadk_display_push_rect_uniform(
            (eadk_rect_t) {
                .x = res * i,
                .y = (EADK_SCREEN_HEIGHT - wall_height) / 2 + wall_height,
                .width = res,
                .height = (uint16_t)ceilf((EADK_SCREEN_HEIGHT - wall_height) / 2.0),
            },
            eadk_color_black
        );
    }
}