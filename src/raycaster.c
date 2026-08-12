#include "raycaster.h"
#include "maze.h"
#include "math.h"
#include "texture_loader.h"
#include "rendering.h"
#include "fixed_points.h"
#include <assert.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>

// lookup table
Y_ToDepthLookup depth_y_lookup = {
    .offset_y = 0xffff, // extremly unlikly for the first y offset
    .table    = {0},
};

// Yes I actually know that much
#define PI 3.141592653589793f

// don't draw entities that are at a depth less than the near camera plane
#define NEAR_CAM_DEPTH (0.05f)

HitInfo raycast_single_ray(Ray ray, Maze *maze) {
    // thank to https://lodev.org/cgtutor/raycasting.html
    // for the algorithms
    
    #define FLOOR_i16(F) (int16_t)F
    point_t map_pos = { FLOOR_i16(ray.origine.x), FLOOR_i16(ray.origine.y) };

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

        // If is ray going outside of the maze
        if (map_pos.x >= maze->width || map_pos.y >= maze->height || map_pos.x < 0 || map_pos.y < 0) {
            break;
        }

        // compute the tile type
        tile_idx = map_pos.x + map_pos.y * maze->width;

        // If hite a block
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
    if (hit_side == VERTICAL   && ray.direction.x > 0) tex_x = TEXTURE_WIDTH - tex_x - 1;
    if (hit_side == HORIZONTAL && ray.direction.y < 0) tex_x = TEXTURE_WIDTH - tex_x - 1;

    // return all these infos in an hitInfo
    return (HitInfo) {
        .distance = distance,
        .side     = hit_side,
        .block_type = block_type,
        .texture_x  = tex_x & (TEXTURE_WIDTH - 1),
        .hit_x      = wall_x,
        .map_pos    = map_pos,
    };
}

/**
 * get the texture for any block type
 * @note default texture 0
 */
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

/**
 * draw the vertical slice of a wall
 * @warning assums vertical_buffer and vertical_depth are initialized properly
 */
static void draw_wall_and_floor_slice(color_t *vertical_buffer, float *vertical_depth, Ray ray, HitInfo hitInfo, int16_t offset_y, textures_t textures) {
    if (vertical_buffer == NULL || vertical_depth == NULL) {
        return;
    }
    
    // compute where is gonna be the center of the wall on the screen
    int16_t wall_center = SCREEN_HEIGHT / 2 + offset_y / hitInfo.distance;

    // compute the height of the wall
    int16_t wall_height = (int16_t)(SCREEN_HEIGHT / hitInfo.distance);

    // compute the highest point on this wall slice
    int16_t wall_top   = wall_center - wall_height / 2;
    int16_t draw_start = wall_top;
    if (draw_start < 0) draw_start = 0;

    // compute the last point on the wall
    uint16_t draw_end = wall_center + wall_height / 2;
    if (draw_end > SCREEN_HEIGHT) draw_end = SCREEN_HEIGHT;

    // compute the step of the texture y
    float delta_texture_y = TEXTURE_HEIGHT / (float)wall_height;
    float texture_y       = (draw_start - wall_top) * delta_texture_y;

    // grab the current texture
    color_t *current_texture = map_block_type_to_texture(hitInfo.block_type, textures);

    // increament the texture_y by it's delta at the same time as y
    for (uint16_t y = draw_start; y < draw_end; y++, texture_y += delta_texture_y) {
        if (vertical_depth[y] < hitInfo.distance) {
            continue;
        }

        // get the color from the texture with avoiding overflows
        uint8_t tex_y = (uint8_t)texture_y % TEXTURE_HEIGHT;
        color_t color = current_texture[hitInfo.texture_x * TEXTURE_HEIGHT + tex_y];

        // darken the HORIZONTAL side
        if (hitInfo.side == HORIZONTAL) color = (color >> 1) & 0xfbef;

        // write the color
        vertical_buffer[y] = color;
        vertical_depth [y] = hitInfo.distance;
    }

    // thanks to https://lodev.org/cgtutor/raycasting2.html for the tip
    if (offset_y != depth_y_lookup.offset_y) {
        // shall refresh the lookup table
        for (int16_t y = 0; y < SCREEN_HEIGHT; y++) {
            if (y == SCREEN_HEIGHT/2) continue; //dst = INFINITY
            depth_y_lookup.table[y] = float_to_fixed16((offset_y + SCREEN_HEIGHT/2) / (float)(y - SCREEN_HEIGHT/2));
        }

        depth_y_lookup.offset_y = offset_y;
    }

    // draw the floor
    fixed16_t floor_x_wall, floor_y_wall;

    // 4 different wall directions possible
    if (hitInfo.side == VERTICAL && ray.direction.x > 0)
    {
        floor_x_wall = INT_TO_FIXED16(hitInfo.map_pos.x);
        floor_y_wall = INT_TO_FIXED16(hitInfo.map_pos.y) + float_to_fixed16(hitInfo.hit_x);
    }
    else if (hitInfo.side == VERTICAL && ray.direction.x < 0)
    {
        floor_x_wall = INT_TO_FIXED16(hitInfo.map_pos.x + 1);
        floor_y_wall = INT_TO_FIXED16(hitInfo.map_pos.y) + float_to_fixed16(hitInfo.hit_x);
    }
    else if (hitInfo.side == HORIZONTAL && ray.direction.y > 0)
    {
        floor_x_wall = INT_TO_FIXED16(hitInfo.map_pos.x) + float_to_fixed16(hitInfo.hit_x);
        floor_y_wall = INT_TO_FIXED16(hitInfo.map_pos.y);
    }
    else
    {
        floor_x_wall = INT_TO_FIXED16(hitInfo.map_pos.x) + float_to_fixed16(hitInfo.hit_x);
        floor_y_wall = INT_TO_FIXED16(hitInfo.map_pos.y + 1);
    }

    // set the first texture for the floor
    color_t *floor_texture = textures.wall_textures + TEXTURE_SIZE*6;
    color_t *ceiling_texture = textures.wall_textures + TEXTURE_SIZE;

    if (draw_end < 0) draw_end = SCREEN_HEIGHT;
    fixed16_t inv_dst = float_to_fixed16(1 / hitInfo.distance);

    // constants for speed
    fixed16_t origine_x = float_to_fixed16(ray.origine.x);
    fixed16_t origine_y = float_to_fixed16(ray.origine.y);
    fixed16_t dx = floor_x_wall - origine_x;
    fixed16_t dy = floor_y_wall - origine_y;

    for (int16_t y = draw_end+1; y < SCREEN_HEIGHT; y++) {
        // current distance to pixel
        fixed16_t floor_dst = depth_y_lookup.table[y];

        // if the depth of this pixel is already less don't render there is some thing in front
        if (vertical_depth[y] < fixed16_to_float(floor_dst)) {
            continue;
        }

        // compute the current weight (distance in range [0, wall_distance])
        fixed16_t weight = fixed16_mul(floor_dst, inv_dst);

        // find the texture pos in range [0, 1]
        fixed16_t current_floor_pos_x = origine_x + fixed16_mul(weight, dx);
        fixed16_t current_floor_pos_y = origine_y + fixed16_mul(weight, dy);

        // change from range [0, 1] to [0, TEXTURE_WIDTH] and [0, TEXTURE_HEIGHT]
        int32_t texture_x = FIXED16_TO_INT(fixed16_mul(FRAC_f16(current_floor_pos_x), INT_TO_FIXED16(TEXTURE_WIDTH)));
        int32_t texture_y = FIXED16_TO_INT(fixed16_mul(FRAC_f16(current_floor_pos_y), INT_TO_FIXED16(TEXTURE_HEIGHT)));
        
        // floor
        // texture laid down on the side
        vertical_buffer[y] = floor_texture[texture_x * TEXTURE_HEIGHT + texture_y];
        vertical_depth [y] = fixed16_to_float(floor_dst);

        // ceiling
        vertical_buffer[SCREEN_HEIGHT - y] = ceiling_texture[texture_x * TEXTURE_HEIGHT + texture_y];
        vertical_depth [SCREEN_HEIGHT - y] = fixed16_to_float(floor_dst);
    }
}

static void draw_entity_slice(color_t *vertical_buffer, float *vertical_depth, EntitySlice entity, int16_t offset_y, textures_t textures) {
    if (vertical_buffer == NULL || vertical_depth == NULL) {
        return;
    }

    // compute entity box
    int16_t entity_height = abs((int16_t)(SCREEN_HEIGHT / (1.2f * entity.dst)));
    
    // compute the rate of change of the entity_texture_y (needed to plant the entitiy's feet on the ground)
    float delta_entity_texture_y = textures.entities_tex_h / (float)entity_height;

    // for the entity to be on the ground
    int16_t entity_draw_end_y   = (SCREEN_HEIGHT/2) + (SCREEN_HEIGHT/2 + offset_y)/(entity.dst) + delta_entity_texture_y * 3;
    int16_t entity_high_y       = entity_draw_end_y - entity_height;
    int16_t entity_draw_start_y = entity_high_y;

    // clamp draw_start and draw_end
    if (entity_draw_start_y < 0) entity_draw_start_y = 0;
    if (entity_draw_end_y > SCREEN_HEIGHT) entity_draw_end_y = SCREEN_HEIGHT;

    // compute stating texture y coord
    float entity_texture_y = (entity_draw_start_y - entity_high_y) * delta_entity_texture_y;

    for (int16_t y = entity_draw_start_y; y < entity_draw_end_y; y++, entity_texture_y += delta_entity_texture_y) {
        // check if there is no thing in front of the entity
        if (vertical_depth[y] < entity.dst) {
            continue;
        }

        // contrain the texture_y inside and sample the texture
        int8_t tex_y = (uint8_t)entity_texture_y % textures.entities_tex_h;
        color_t color = textures.entities_textures[tex_y + entity.texture_x * textures.entities_tex_h];

        // white is considered transparent
        if (color == eadk_color_white) continue;

        // draw the entity
        vertical_buffer[y] = color;
        vertical_depth [y] = entity.dst;
    }
}

/**
 * renders the current slice to the screen
 * @warning vertical_buffer assumed of length SCREEN_HEIGHT
 */
static void render_vertical_slice_to_screen(color_t *vertical_buffer, uint16_t x) {
    if (vertical_buffer == NULL) {
        return;
    }

    // draw it to the screen
    draw_rect_textured(
        (eadk_rect_t) {
            .x = x,
            .y = 0,
            .width = 1,
            .height = SCREEN_HEIGHT,
        },
        vertical_buffer
    );
}

static inline bool in_rect(rect_t rect, uint16_t x, uint16_t y) {
    return x >= rect.x && x <= rect.x + rect.width && y >= rect.y && y <= rect.y + rect.height;
}

static int16_t in_any_rect(LayeredTextures layers, uint16_t x, uint16_t y) {
    for (uint16_t i = 0; i < layers.num_rects; i++)
        if (in_rect(layers.rects[i], x, y)) return i;
    return -1;
}

/**
 * clear both vertical buffers the color and depth channels
 * @note any null will be ignored
 */
static void clear_vertical_buffers(color_t *vertical_buffer, float *vertical_depth, uint16_t x, LayeredTextures layers) {
    if (vertical_buffer == NULL || vertical_depth == NULL) return;

    for (uint16_t y = 0; y < SCREEN_HEIGHT; y++) {
        int16_t i = in_any_rect(layers, x, y);

        if (i == -1) {
            vertical_buffer[y] = 0;
            vertical_depth[y] = INFINITY;
            continue;
        }
        
        if (layers.pixels_by_rect[i] == NULL) {
            vertical_buffer[y] = 0;
        }
        else {
            // local xy
            uint16_t local_x = x - layers.rects[i].x;
            uint16_t local_y = y - layers.rects[i].y;
            uint16_t pix_index = (layers.on_the_side[i])? 
                    local_x * layers.rects[i].height + local_y 
                    : local_x + local_y * layers.rects[i].width;

            // set color
            vertical_buffer[y] = layers.pixels_by_rect[i][pix_index];
        }

        // depth set to negative layer
        vertical_depth[y] = -layers.layers[i];
    }
}

void raycast_render(Player player, Maze *maze, Entity *entities, size_t num_entities, textures_t textures, LayeredTextures layers) {

    // compute the depth and the slice of all entities
    EntityDepth entities_depth[SCREEN_WIDTH];
    get_entities_depth(player, entities_depth, entities, num_entities, textures);

    // I can't have an entire backbuffer for the whole screen
    // so I keep a backbuffer per column since that is pretty
    // good for a raycaster that renders per column
    float   vertical_depth [SCREEN_HEIGHT];
    color_t vertical_buffer[SCREEN_HEIGHT];

    // raycast
    for (uint16_t i = 0; i < SCREEN_WIDTH; i++) {
        // clear the vertical buffers
        clear_vertical_buffers(vertical_buffer, vertical_depth, i, layers);

        // x-coordinate in camera space
        float camera_x = 2 * i / (float)SCREEN_WIDTH - 1; 

        // compute direction of the ray
        Vec2 dir = {
            player.dir.x + player.plane.x * camera_x, 
            player.dir.y + player.plane.y * camera_x
        };

        // create the ray starting from the player's position and going in the direction computed above 
        Ray ray = (Ray) { player.pos, dir };

        // cast the ray
        HitInfo hitInfo = raycast_single_ray(ray, maze);

        // draw the wall and the floor
        int16_t y_offset = 0; // offset Y of the player compared to the center of the screen
        draw_wall_and_floor_slice(vertical_buffer, vertical_depth, ray, hitInfo, y_offset, textures);

        // recompute wall_enter for caching
        int16_t wall_center = SCREEN_HEIGHT / 2 + y_offset / hitInfo.distance;
        if (wall_center < 0) wall_center = 0;
        if (wall_center >= SCREEN_HEIGHT) wall_center = SCREEN_HEIGHT;

        // If the closest entity is in front of the wall then draw all entities in that slice
        if (vertical_depth[wall_center] <= entities_depth[i].min_depth) {
            // avoid nesting using goto
            goto ignore_rendering_entities;
        }

        // draw all entities
        for (uint16_t entity = 0; entity < entities_depth[i].num_entities; entity++) {
            draw_entity_slice(vertical_buffer, vertical_depth, entities_depth[i].entities[entity], 0, textures);
        }

ignore_rendering_entities:
        // render the current slice (like swapping the buffers but I can't do that since I have not enought memory)
        render_vertical_slice_to_screen(vertical_buffer, i);
        
        // free all the entities infos for that slice since they are outdated
        if (entities_depth[i].entities != NULL) {
            free(entities_depth[i].entities);
        }
    }
}

void get_entities_depth(Player player, EntityDepth *entities_depth, Entity *entities, size_t num_entities, textures_t textures) {
    // thanks to https://lodev.org/cgtutor/raycasting3.html

    void push_entity_to_depth(EntityDepth *depth, EntitySlice slice);

    // the entities depth for each screen slice is initialized to a NULL value
    for (uint16_t i = 0; i < SCREEN_WIDTH; i++) {
        entities_depth[i] = (EntityDepth) {
            .min_depth = INFINITY,
            .num_entities = 0,
            .capacity = 0,
            .entities = NULL,
        };
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

        // transform sprite with the inverse camera matrix:
        // [ planeX   dirX ] -1                                       [ dirY      -dirX ]
        // [               ]       =  1/(planeX*dirY-dirX*planeY) *   [                 ]
        // [ planeY   dirY ]                                          [ -planeY  planeX ]

        Vec2 transformed_pos = {
            inv_det * ( player.dir.y   * entity_pos.x - player.dir.x   * entity_pos.y),
            inv_det * (-player.plane.y * entity_pos.x + player.plane.x * entity_pos.y), // Y is actually the depth
        };
        
        // If behind the camera
        if (transformed_pos.y <= NEAR_CAM_DEPTH) {    
            continue;
        }

        // compute the x position on the screen and the width
        int16_t entity_screen_pos_x = (int16_t)SCREEN_WIDTH / 2 * (1 + transformed_pos.x / transformed_pos.y);
        int16_t entity_width =  (int16_t)SCREEN_HEIGHT / (2.0f * transformed_pos.y); // transform_pos.y > NEAR_CAM_DEPTH > 0

        // check for overflows
        if ( entity_screen_pos_x - entity_width / 2 >= SCREEN_WIDTH 
            || entity_screen_pos_x + entity_width / 2 < 0           ) {
            continue;
        }

        // compute start and end of the draw on the x axis
        int16_t draw_start_x = entity_screen_pos_x - entity_width / 2;
        int16_t draw_end_x   = draw_start_x + entity_width;
        if (draw_start_x < 0) draw_start_x = 0;
        if (draw_end_x > SCREEN_WIDTH) draw_end_x = SCREEN_WIDTH;

        // texture x computation
        float delta_texture_x = textures.entities_tex_w / (float)entity_width;
        float texture_x       = (draw_start_x - entity_screen_pos_x + entity_width / 2) * delta_texture_x;

        // for all slice_x push the entity the slice corresponding to the x value
        for (int16_t slice = draw_start_x; slice < draw_end_x; slice++, texture_x += delta_texture_x) {
            push_entity_to_depth(
                &entities_depth[slice], 
                (EntitySlice) { .type = entity_type, .texture_x = (uint8_t)texture_x, .dst = transformed_pos.y }
            );
        }
    }
}

void push_entity_to_depth(EntityDepth *depth, EntitySlice slice) {
    // avoid overflowing the number of entities
    if (depth->num_entities == 255) return;
    if (depth->num_entities + 1 <= depth->capacity) {
        // a goto statment? yes it is the only kind of way that the goto statment is useful
        // since this is not the most interessting path a goto statment avoid having a big 
        // thing inside of an If statment.
        goto directly_push_entity;
    }

    // the buffer has not enough place so we need to allocate more space
    // in order to have the perfect traidoff we reallocate about twice what we need
    // so that we don't have to reallocate any time soon.
    uint16_t new_capacity = (depth->capacity + 1) * 2;
    if (new_capacity > 255) new_capacity = 255;

    // reallocate the entity buffer
    EntitySlice *new_entities = realloc(depth->entities, new_capacity * sizeof(EntitySlice));

    // allocation failed
    // TODO: unwind execution and show error screen
    if (new_entities == NULL) return;

    // put the entity array back in the entity depth struct
    depth->entities = new_entities; // update the pointer
    depth->capacity = new_capacity; // update the capacity

directly_push_entity:
    // push the entity to the entity vector
    depth->entities[depth->num_entities++] = slice;

    // update the min_depth if the depth is below the last recorded
    if (slice.dst < depth->min_depth) {
        depth->min_depth = slice.dst;
    }
}
