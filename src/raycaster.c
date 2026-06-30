#include "raycaster.h"
#include "maze.h"
#include "math.h"

#include <stdio.h>

// Yes I actually know that much
#define PI 3.141592653589793f

HitInfo raycast_single_ray(Ray ray, Maze *maze) {
    // thank to https://lodev.org/cgtutor/raycasting.html
    // for the algorithm
    
    #define FLOOR_u16(F) (uint16_t)F
    eadk_point_t map_pos = { FLOOR_u16(ray.origine.x), FLOOR_u16(ray.origine.y) };

    int8_t step_x, step_y;
    float side_distance_x, side_distance_y;

    if (ray.direction.x == 0) {
        step_x = 0;
        side_distance_x = INFINITY;
    }
    else {
        // ray.direction.x < 0 so |ray.direction.x| = -ray.direction.x
        // else |ray.direction.x| = ray.direction.x

        // the equations are thus
        // (ray.origine.x - map_pos.x) / -ray.direction.x <=> (map_pos.x - ray.origine.x) / ray.direction.x, ray.direction.x < 0
        // (map_pos.x + 1 - ray.origine.x) / ray.direction.x, ray.direction.x > 0
        // And we know that the expression ray.direction.x > 0 will evaluate to 1 if and only if ray.direction.x > 0
        // so the final expression ends up being what is below

        uint8_t dir_positive = ray.direction.x > 0;
        step_x = dir_positive? 1 : -1;
        side_distance_x = (map_pos.x - ray.origine.x + (float)dir_positive) / ray.direction.x;
    }

    if (ray.direction.y == 0) {
        step_y = 0;
        side_distance_y = INFINITY;
    }
    else {
        // ray.direction.y < 0 so |ray.direction.y| = -ray.direction.y
        // else |ray.direction.y| = ray.direction.y

        // the equations are thus
        // (ray.origine.y - map_pos.y) / -ray.direction.y <=> (map_pos.y - ray.origine.y) / ray.direction.y, ray.direction.y < 0
        // (map_pos.y + 1 - ray.origine.y) / ray.direction.y, ray.direction.y > 0
        // And we know that the eypression ray.direction.y > 0 will evaluate to 1 if and only if ray.direction.y > 0
        // so the final eypression ends up being what is below

        uint8_t dir_positive = ray.direction.y > 0;
        step_y = dir_positive? 1 : -1;
        side_distance_y = (map_pos.y - ray.origine.y + (float)dir_positive) / ray.direction.y;
    }

    // if dir.x < 0 then step_x == -1 so step_x * ray.direction.x = |ray.direction.x|
    // the same logic applies for y
    float delta_distance_x = (step_x == 0)? INFINITY : 1 / (step_x * ray.direction.x);
    float delta_distance_y = (step_y == 0)? INFINITY : 1 / (step_y * ray.direction.y);

    float distance = 0.0f;
    Side hit_side  = UNDEFINED_SIDE;
    uint16_t tile_idx;

    // does not look like but is a really fast loop
    // not float operation other than addition
    // and a really fast algorithm
    
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
                .side = hit_side,
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
    float field_of_view = PI / 3;

    for (int i = 0; i <= EADK_SCREEN_WIDTH / res; i++) {
        float t = (float)i / EADK_SCREEN_WIDTH * res;
        float angle = (player_angle - field_of_view * 0.5f) + t * (field_of_view);

        // garentee to have length 1 then apply correction on distance to avoid fish eye len effect
        Vec2 dir = (Vec2) { cosf(angle), sinf(angle) };
        HitInfo hitInfo = raycast_single_ray((Ray) { pos, dir }, maze);
        float distance = hitInfo.distance * cosf(player_angle - angle);

        uint16_t wall_height = (uint16_t)(300.0 / distance);

        if (wall_height >= EADK_SCREEN_HEIGHT) {
            wall_height = EADK_SCREEN_HEIGHT;
        }

        eadk_color_t color;
        switch (hitInfo.block_type) {
            case 1:
                color = (hitInfo.side == HORIZONTAL)? eadk_color_blue : eadk_color_blue - 5;
                break;
            
            default:
                color = (hitInfo.side == HORIZONTAL)? eadk_color_red : eadk_color_red - (5 << 11);
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