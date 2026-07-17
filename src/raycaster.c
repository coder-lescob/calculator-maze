#include "raycaster.h"
#include "maze.h"
#include "math.h"
#include "textures.h"
#include "rendering.h"
#include <string.h>

#include <stdio.h>
#include <stdlib.h>

// Yes I actually know that much
#define PI 3.141592653589793f

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

        // compute the tile index
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
    if(hit_side == 0 && ray.direction.x > 0) tex_x = TEXTURE_WIDTH - tex_x - 1;
    if(hit_side == 1 && ray.direction.y < 0) tex_x = TEXTURE_WIDTH - tex_x - 1;

    return (HitInfo) {
        .distance = distance,
        .side     = hit_side,
        .block_type = block_type,
        .texture_x  = tex_x & (TEXTURE_WIDTH - 1),
    };
}

static eadk_color_t *map_block_type_to_texture(uint8_t block_type) {
    switch (block_type) {
        case 0:
            return textures + TEXTURE_SIZE;

        case 1:
            return textures;

        default:
            return textures;
    }
}

static void draw_vertical_texture_strip(uint16_t x, uint16_t wall_height, HitInfo hitInfo) {
    // compute the highest point on this wall slice
    int16_t draw_start = (EADK_SCREEN_HEIGHT - wall_height) / 2;
    if (draw_start < 0) draw_start = 0;

    // compute the last point on the wall
    uint16_t draw_end = wall_height + draw_start;
    if (draw_end > EADK_SCREEN_HEIGHT) draw_end = EADK_SCREEN_HEIGHT;

    // compute the step of the texture y
    float delta_texture_y = TEXTURE_HEIGHT / (float)wall_height;
    float texture_y       = (draw_start - EADK_SCREEN_HEIGHT / 2 + wall_height / 2) * delta_texture_y;

    // create a buffer for the height of the screen
    /**
     * TODO: maybe add floor and ceiling
     */
    eadk_color_t line_buffer[EADK_SCREEN_HEIGHT];
    memset(line_buffer, 0, EADK_SCREEN_HEIGHT * sizeof(eadk_color_t));

    // get the current texture
    eadk_color_t *current_texture = map_block_type_to_texture(hitInfo.block_type);

    for (uint16_t y = draw_start; y < draw_end; y++) {
        // get the color from the texture with avoiding overflows
        uint8_t tex_y = (uint8_t)texture_y & (TEXTURE_HEIGHT - 1);
        eadk_color_t color = current_texture[hitInfo.texture_x * TEXTURE_HEIGHT + tex_y];

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
            .height = EADK_SCREEN_HEIGHT,
        },
        line_buffer
    );
}

void raycast_render(Player player, Maze *maze, Entity *entities, size_t num_entities) {
    float field_of_view = PI / 3;

    float depth_buffer[EADK_SCREEN_WIDTH + 1];

    for (int i = 0; i <= EADK_SCREEN_WIDTH; i++) {
        float t = (float)i / EADK_SCREEN_WIDTH;
        float angle = (player.angle - field_of_view * 0.5f) + t * (field_of_view);

        // garentee to have length 1
        Vec2 dir = (Vec2) { cosf(angle), sinf(angle) };
        HitInfo hitInfo = raycast_single_ray((Ray) { player.pos, dir }, maze);

        // save the distance to the depth buffer
        float distance = hitInfo.distance;
        depth_buffer[i] = distance;

        // compute the wall height
        uint16_t wall_height = (uint16_t)(EADK_SCREEN_HEIGHT / distance);
        draw_vertical_texture_strip(i, wall_height, hitInfo);
    }
    
    draw_entities(player, depth_buffer, entities, num_entities);
}

void draw_entities(Player player, float *depth_buffer, Entity *entities, size_t num_entities) {
    // rganks to https://lodev.org/cgtutor/raycasting3.html

    // compute player dir
    float dirX   =  cosf(player.angle);
    float dirY   =  sinf(player.angle);

    // the plane is 90° away from the direction and scaled by tan fov/2
    float planeX = -dirY * 0.57735026919f; // tan fov/2
    float planeY =  dirX * 0.57735026919f; // tan fov/2

    // precompute the cpnstant factor for the inverse camera matrix
    // [ planeX   dirX ] -1                                       [ dirY      -dirX ]
    // [               ]       =  1/(planeX*dirY-dirX*planeY) *   [                 ]
    // [ planeY   dirY ]                                          [ -planeY  planeX ]
    float inv_det = 1.0f / (planeX * dirY - dirX * planeY);

    // sort the entities
    uint16_t entity_order[num_entities];
    sort_entities_by_distance(player, num_entities, entities, entity_order);

    for (uint16_t i = 0; i < num_entities; i++) {
        // get the type of the entity
        uint8_t entity_type = entities[entity_order[i]].entity_type;

        // relative pos
        Vec2 entity_pos = {
            entities[entity_order[i]].pos.x - player.pos.x,
            entities[entity_order[i]].pos.y - player.pos.y
        };

        // transform sprite with the inverse camera matrix
        // [ planeX   dirX ] -1                                       [ dirY      -dirX ]
        // [               ]       =  1/(planeX*dirY-dirX*planeY) *   [                 ]
        // [ planeY   dirY ]                                          [ -planeY  planeX ]

        Vec2 transformed_pos = {
            inv_det * ( dirY * entity_pos.x   - dirX * entity_pos.y),
            inv_det * (-planeY * entity_pos.x + planeX * entity_pos.y), // Y is actually the depth
        };

        if (transformed_pos.y <= 0) {
            // behind the camera
            continue;
        }

        uint16_t entity_screen_pos_x = (uint16_t)(EADK_SCREEN_WIDTH / 2 * (1 + transformed_pos.x / transformed_pos.y));
        uint16_t entity_width =  abs((uint16_t)(EADK_SCREEN_HEIGHT / (3.0f * transformed_pos.y)));
        uint16_t entity_height = abs((uint16_t)(EADK_SCREEN_HEIGHT / (1.5f * transformed_pos.y)));

        // compuet start and end
        int16_t draw_start_x = entity_screen_pos_x - entity_width / 2;
        int16_t draw_end_x   = draw_start_x + entity_width;
        if (draw_start_x < 0) draw_start_x = 0;
        if (draw_end_x > EADK_SCREEN_WIDTH) draw_end_x = EADK_SCREEN_WIDTH;
        
        // for the entity to be on the ground
        // draw_end_y = EADK_SCREEN_HEIGHT * (transformed_pos.y - 1) / (2 * transformed_pos.y), draw_start_y - draw_end_y = entity_height
        // draw_start_y = EADK_SCREEN_HEIGHT * (transformed_pos.y + 1) / (2 * transformed_pos.y) - entity_height
        int16_t draw_start_y = EADK_SCREEN_HEIGHT * (transformed_pos.y + 1) / (2 * transformed_pos.y) - entity_height;
        int16_t draw_end_y   = draw_start_y + entity_height; 
        if (draw_start_y < 0) draw_start_y = 0;
        if (draw_end_y > EADK_SCREEN_HEIGHT) draw_end_y = EADK_SCREEN_HEIGHT;

        for (int16_t strip = draw_start_x; strip <= draw_end_x; strip++) {
            if (depth_buffer[strip] <= transformed_pos.y) {
                continue;
            }

            draw_rect_uniform(
                (eadk_rect_t) {
                    .x = strip,
                    .y = draw_start_y,
                    .width = 1,
                    .height = (draw_end_y - draw_start_y) - 1,
                },
                (entity_type == 0)? eadk_color_red : eadk_color_blue
            );
        }
    }
}

void sort_entities_by_distance(Player player, size_t num_entities, Entity *entities, uint16_t *entity_order) {
    struct EntityEntry entities_table[num_entities];
    int cmp_entity_by_dst(const void *a, const void *b);

    for (int i = 0; i < num_entities; i++) {
        // compute relative pos
        Vec2 entity_pos = {
            entities[i].pos.x - player.pos.x,
            entities[i].pos.y - player.pos.y
        };

        // compute the ditance to the player
        float sqr_distance_to_player = entity_pos.x * entity_pos.x + entity_pos.y * entity_pos.y;
        entities_table[i] = (struct EntityEntry) { .index = i, .dst = sqr_distance_to_player };
    }

    // sort them
    qsort(entities_table, num_entities, sizeof(struct EntityEntry), cmp_entity_by_dst);

    // put them in entity_order to return the value
    for (uint16_t i = 0; i < num_entities; i++) {
        entity_order[i] = entities_table[i].index;
    }
}

int cmp_entity_by_dst(const void *a, const void *b) {
    // get the entries
    struct EntityEntry *entry_a = (struct EntityEntry *)a;
    struct EntityEntry *entry_b = (struct EntityEntry *)b;

    // account for tiny distances by not doing subtraction
    if (entry_a->dst > entry_b->dst) {
        // swap them
        return -1;
    }
    else if (entry_a->dst < entry_b->dst) {
        // nice please don't swap
        return 1;
    }

    // the same who cares ?!
    return 0;
}