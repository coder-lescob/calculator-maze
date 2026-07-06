#include "raycaster.h"
#include "maze.h"
#include "math.h"
#include "textures.h"

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

    Side hit_side  = UNDEFINED_SIDE;
    uint16_t tile_idx;

    // does not look like but is a really fast loop
    // not float operation other than addition
    // and a really fast algorithm

    uint16_t block_type = 0;
    
    while (true) {
        if (side_distance_x < side_distance_y) {
            side_distance_x += delta_distance_x;
            map_pos.x       += step_x;
            hit_side         = VERTICAL;
        }
        else {
            side_distance_y += delta_distance_y;
            map_pos.y       += step_y;
            hit_side         = HORIZONTAL;
        }

        if (map_pos.x >= maze->width || map_pos.y >= maze->height) {
            break;
        }

        // compute the tile index
        tile_idx = map_pos.x + map_pos.y * maze->width;

        if (maze->tiles[tile_idx] > 0) {
            block_type = maze->tiles[tile_idx];
            break;
        }
    }

    float distance;

    // Calculate distance projected on camera direction (Euclidean distance would give fisheye effect!)
    if(hit_side == 0) distance = (side_distance_x - delta_distance_x);
    else              distance = (side_distance_y - delta_distance_y);

    //calculate value of wallX
    float wallX; //where exactly the wall was hit
    if (hit_side == 0) wallX = ray.origine.y + distance * ray.direction.y;
    else               wallX = ray.origine.x + distance * ray.direction.x;
    wallX -= (int)wallX;

    //x coordinate on the texture
    int tex_x = (int)(wallX * (float)TEXTURE_WIDTH);
    if(hit_side == 0 && ray.direction.x > 0) tex_x = TEXTURE_WIDTH - tex_x - 1;
    if(hit_side == 1 && ray.direction.y < 0) tex_x = TEXTURE_WIDTH - tex_x - 1;

    return (HitInfo) {
        .distance = distance,
        .side = hit_side,
        .block_type = block_type,
        .texture_x = tex_x,
    };
}

static eadk_color_t *map_block_type_to_texture(uint8_t block_type) {
    switch (block_type) {
        case 0:
            return textures + TEXTURE_SIZE * 5;

        case 1:
            return textures + TEXTURE_SIZE * 3;

        default:
            return textures + TEXTURE_SIZE * 3;
    }
}

static void draw_vertical_texture_strip(uint16_t res, uint16_t x, uint16_t wall_height, HitInfo hitInfo) {
    int16_t draw_y = (EADK_SCREEN_HEIGHT - wall_height) / 2;
    if (draw_y < 0) draw_y = 0;

    // compute the step of the texture y
    float delta_texture_y = TEXTURE_HEIGHT / (float)wall_height;
    float texture_y       = (draw_y - EADK_SCREEN_HEIGHT / 2 + wall_height / 2) * delta_texture_y;

    if (wall_height + draw_y > EADK_SCREEN_HEIGHT) {
        // we need that wall_height + draw_y = EADK_SCREEN_HEIGHT
        wall_height = EADK_SCREEN_HEIGHT - draw_y;
    }

    eadk_color_t line_buffer[wall_height];
    eadk_color_t *current_texture = map_block_type_to_texture(hitInfo.block_type);

    for (uint16_t y = 0; y < wall_height; y++) {
        // get the color from the texture
        eadk_color_t color = current_texture[hitInfo.texture_x * TEXTURE_HEIGHT + (uint8_t)texture_y];

        // darken the HORIZONTAL side
        if (hitInfo.side == HORIZONTAL) color = (color >> 1) & 0xfbef;

        // write the color
        line_buffer[y] = color;

        // increament the texture_y by it's delta
        texture_y += delta_texture_y;
    }

    // draw it to the screen
    eadk_display_push_rect(
        (eadk_rect_t) {
            .x = x,
            .y = draw_y,
            .width = res,
            .height = wall_height,
        },
        line_buffer
    );
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
        float distance = hitInfo.distance;

        uint16_t wall_height = (uint16_t)(EADK_SCREEN_HEIGHT / distance);

        draw_vertical_texture_strip(res, res * i, wall_height, hitInfo);

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
                .y = (EADK_SCREEN_HEIGHT + wall_height) / 2,
                .width = res,
                .height = (uint16_t)ceilf((EADK_SCREEN_HEIGHT - wall_height) / 2.0),
            },
            eadk_color_black
        );
    }
}