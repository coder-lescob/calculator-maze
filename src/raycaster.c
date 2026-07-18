#include "raycaster.h"
#include "maze.h"
#include "math.h"
#include "texture_loader.h"
#include "rendering.h"
#include <string.h>

#include <stdio.h>
#include <stdlib.h>

// Yes I actually know that much
#define PI 3.141592653589793f

#define NEAR_CAM_DEPTH (0.05f)

HitInfo raycast_single_ray(Ray ray, Maze *maze) {
    // thank to https://lodev.org/cgtutor/raycasting.html
    // for the algorithms
    
    #define FLOOR_u16(F) (uint16_t)F
    eadk_point_t map_pos = { FLOOR_u16(ray.origine.x), FLOOR_u16(ray.origine.y) };

    int8_t step_x, step_y;
    float side_distance_x, side_distance_y;

    if (ray.direction.x == 0) {
        step_x = 0;
        side_distance_x = INFINITY;
    }
    else {
        // if ray.direction.x < 0 so |ray.direction.x| = -ray.direction.x
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
        // if ray.direction.y < 0 so |ray.direction.y| = -ray.direction.y
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

        // compute the tile type
        tile_idx = map_pos.x + map_pos.y * maze->width;

        if (maze->tiles[tile_idx] > 0) {
            block_type = maze->tiles[tile_idx];
            break;
        }
    }

    // Calculate distance projected on camera direction (Euclidean distance would give fisheye effect!)
    float distance;
    if (hit_side == VERTICAL) distance = (side_distance_x - delta_distance_x);
    else                      distance = (side_distance_y - delta_distance_y);

    // calculate value of wall_x
    float wall_x; // where exactly the wall was hit
    if (hit_side == VERTICAL) wall_x = ray.origine.y + distance * ray.direction.y;
    else                      wall_x = ray.origine.x + distance * ray.direction.x;
    wall_x -= (int)wall_x;

    // x coordinate on the texture use bilinear interpolation
    int tex_x = (int)(wall_x * (float)TEXTURE_WIDTH);
    if(hit_side == VERTICAL   && ray.direction.x > 0) tex_x = TEXTURE_WIDTH - tex_x - 1;
    if(hit_side == HORIZONTAL && ray.direction.y < 0) tex_x = TEXTURE_WIDTH - tex_x - 1;

    return (HitInfo) {
        .distance = distance,
        .side     = hit_side,
        .block_type = block_type,
        .texture_x  = tex_x & (TEXTURE_WIDTH - 1),
    };
}

static color_t *map_block_type_to_texture(uint8_t block_type, textures_t textures) {
    switch (block_type) {
        case 0:
            return textures.wall_textures + TEXTURE_SIZE * 5;

        case 1:
            return textures.wall_textures + TEXTURE_SIZE * 3;

        default:
            return textures.wall_textures;
    }
}

static void draw_vertical_world_slice(uint16_t x, uint16_t sky_x, uint16_t wall_height, HitInfo hitInfo, EntityDepth entity_depth, textures_t textures) {
    int16_t player_height = 20;
    int16_t wall_center = SCREEN_HEIGHT / 2 + player_height / hitInfo.distance;
    int16_t entity_center = SCREEN_HEIGHT / 2 + player_height / entity_depth.dst;

    // compute the highest point on this wall slice
    int16_t draw_start = wall_center - wall_height / 2;
    if (draw_start < 0) draw_start = 0;

    // compute the last point on the wall
    uint16_t draw_end = wall_center + wall_height / 2;
    if (draw_end > SCREEN_HEIGHT) draw_end = SCREEN_HEIGHT;

    // compute the step of the texture y
    float delta_texture_y = TEXTURE_HEIGHT / (float)wall_height;
    float texture_y       = (draw_start - (wall_center - wall_height / 2)) * delta_texture_y;

    // is the entity in front of the wall ?
    bool entity_in_front = entity_depth.dst != INFINITY && entity_depth.dst < hitInfo.distance;

    // entities constants
    int16_t entity_height, entity_draw_start_y, entity_draw_end_y;
    float delta_entity_texture_y, entity_texture_y;

    // compute the entity data only if it is in front
    if (entity_in_front) {
        // compute entity box
        entity_height = abs((int16_t)(SCREEN_HEIGHT / (1.5f * entity_depth.dst)));

        // for the entity to be on the ground
        entity_draw_end_y     = entity_center + SCREEN_HEIGHT / (2 * entity_depth.dst);
        int16_t entity_high_y = entity_draw_end_y - entity_height;
        entity_draw_start_y   = entity_high_y;

        // clamp draw_start and draw_end
        if (entity_draw_start_y < 0) entity_draw_start_y = 0;
        if (entity_draw_end_y > SCREEN_HEIGHT) entity_draw_end_y = SCREEN_HEIGHT;

        // compute texture coords
        delta_entity_texture_y = 0x3f / (float)entity_height;
        entity_texture_y       = (entity_draw_start_y - entity_high_y) * delta_entity_texture_y;
    }

    // create a buffer for the height of the screen
    /**
     * TODO: add floor and sky
     */
    eadk_color_t line_buffer[SCREEN_HEIGHT];
    memcpy(line_buffer, textures.sky_texture + sky_x * SCREEN_HEIGHT / 2, SCREEN_HEIGHT / 2 * sizeof(color_t));
    memset(line_buffer + SCREEN_HEIGHT/2, 0, SCREEN_HEIGHT/2 * sizeof(color_t));

    // get the current texture
    color_t *current_texture = map_block_type_to_texture(hitInfo.block_type, textures);
    int16_t top = (!entity_in_front)? draw_start : (draw_start < entity_draw_start_y)? draw_start : entity_draw_start_y;
    if (top < 0) top = 0;

    for (uint16_t y = top; y < draw_end || (entity_in_front && y < entity_draw_end_y); y++) {
        if (entity_in_front) {
            if (y >= draw_end || y < draw_start || (y >= entity_draw_start_y && y < entity_draw_end_y)) {
                // entity drawing
                uint8_t tex_y = (uint8_t)entity_texture_y & 0x3f;
                color_t color = tex_y << 5 | entity_depth.texture_x << 11;

                line_buffer[y] = color;

                entity_texture_y += delta_entity_texture_y;
                continue;
            }
        }

        // get the color from the texture with avoiding overflows
        uint8_t tex_y = (uint8_t)texture_y & (TEXTURE_HEIGHT - 1);
        color_t color = current_texture[hitInfo.texture_x * TEXTURE_HEIGHT + tex_y];

        // darken the HORIZONTAL side
        if (hitInfo.side == HORIZONTAL) color = (color >> 1) & 0xfbef;

        // write the color
        line_buffer[y] = color;

        // increament the texture_y by it's delta
        texture_y += delta_texture_y;
    }

    // draw it to the screen
    draw_rect_textured(
        (eadk_rect_t) {
            .x = x,
            .y = 0,
            .width = 1,
            .height = SCREEN_HEIGHT,
        },
        line_buffer
    );
}

void raycast_render(Player player, Maze *maze, Entity *entities, size_t num_entities, textures_t textures) {

    // compute the depth and the slice of all entities
    EntityDepth entities_depth[SCREEN_WIDTH];
    get_entities_depth(player, entities_depth, entities, num_entities);

    // raycast
    for (int i = 0; i < SCREEN_WIDTH; i++) {

        // x-coordinate in camera space
        float camera_x = 2 * i / (float)SCREEN_WIDTH - 1; 

        // compute direction of the ray
        Vec2 dir = {
            player.dir.x + player.plane.x * camera_x, 
            player.dir.y + player.plane.y * camera_x
        };

        // cast the ray
        HitInfo hitInfo = raycast_single_ray((Ray) { player.pos, dir }, maze);

        // save the distance to the depth buffer
        float distance = hitInfo.distance;
        int16_t sky_x = i;

        // compute the wall height
        uint16_t wall_height = (uint16_t)(SCREEN_HEIGHT / distance);
        draw_vertical_world_slice(i, sky_x, wall_height, hitInfo, entities_depth[i], textures);
    }
}

void get_entities_depth(Player player, EntityDepth *entities_depth, Entity *entities, size_t num_entities) {
    // thanks to https://lodev.org/cgtutor/raycasting3.html

    // initialize the distances to infinity
    for (uint16_t i = 0; i < SCREEN_WIDTH; i++) {
        entities_depth[i].dst = INFINITY;
    }

    // precompute the constant factor for the inverse camera matrix
    // [ planeX   dirX ] -1                                       [ dirY      -dirX ]
    // [               ]       =  1/(planeX*dirY-dirX*planeY) *   [                 ]
    // [ planeY   dirY ]                                          [ -planeY  planeX ]
    float inv_det = 1.0f / (player.plane.x * player.dir.y - player.dir.x * player.plane.y);

    for (uint16_t i = 0; i < num_entities; i++) {
        // get the type of the entity
        uint8_t entity_type = entities[i].entity_type;

        // relative pos
        Vec2 entity_pos = {
            entities[i].pos.x - player.pos.x,
            entities[i].pos.y - player.pos.y
        };

        // transform sprite with the inverse camera matrix
        // [ planeX   dirX ] -1                                       [ dirY      -dirX ]
        // [               ]       =  1/(planeX*dirY-dirX*planeY) *   [                 ]
        // [ planeY   dirY ]                                          [ -planeY  planeX ]

        Vec2 transformed_pos = {
            inv_det * ( player.dir.y * entity_pos.x   - player.dir.x   * entity_pos.y),
            inv_det * (-player.plane.y * entity_pos.x + player.plane.x * entity_pos.y), // Y is actually the depth
        };

        if (transformed_pos.y <= NEAR_CAM_DEPTH) {
            // behind the camera
            continue;
        }

        int16_t entity_screen_pos_x = (int16_t)(SCREEN_WIDTH / 2 * (1 + transformed_pos.x / transformed_pos.y));
        int16_t entity_width =  abs((int16_t)(SCREEN_HEIGHT / (3.0f * transformed_pos.y)));

        // check for overflows
        if ( entity_screen_pos_x - entity_width / 2 >= SCREEN_WIDTH 
            || entity_screen_pos_x + entity_width / 2 < 0           ) {
            continue;
        }

        // compuet start and end
        int16_t draw_start_x = entity_screen_pos_x - entity_width / 2;
        int16_t draw_end_x   = draw_start_x + entity_width;
        if (draw_start_x < 0) draw_start_x = 0;
        if (draw_end_x > SCREEN_WIDTH) draw_end_x = SCREEN_WIDTH;

        // texture x computation
        float delta_texture_x = 0x1f / (float)entity_width;
        float texture_x       = (draw_start_x - entity_screen_pos_x + entity_width / 2) * delta_texture_x;

        for (int16_t slice = draw_start_x; slice < draw_end_x; slice++) {
            if (entities_depth[slice].dst < transformed_pos.y) {
                // another entity is in front of this slice, don't draw this entity on this slice
                continue;
            }

            entities_depth[slice] = (EntityDepth) {.type = entity_type, .texture_x = (uint8_t)texture_x & 0x1f, .dst = transformed_pos.y };
            texture_x += delta_texture_x;
        }
    }
}