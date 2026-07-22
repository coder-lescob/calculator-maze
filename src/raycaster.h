#ifndef RAYCASTER_H
#define RAYCASTER_H

#include "vec.h"
#include "maze.h"
#include "entity.h"
#include "texture_loader.h"

typedef struct {
    Vec2 origine;
    Vec2 direction;
} Ray;

typedef uint8_t Side;

#define UNDEFINED_SIDE 2
#define VERTICAL 0
#define HORIZONTAL 1

typedef struct {
    float distance;
    Side side;

    uint8_t block_type;
    uint8_t texture_x;

    float hit_x;
    point_t map_pos;
} HitInfo;

typedef struct {
    uint16_t type;
    uint8_t texture_x;
    float dst;
} EntitySlice;

typedef struct {
    float min_depth; // cache the min depth for extra speed
    uint8_t num_entities; // I wont allow more than 255 entities to be viewed through
    uint8_t capacity;
    EntitySlice *entities;
} EntityDepth;

/**
 * @brief a lookup table to avoid doing too much divisons
 * @todo use fixed points
*/
typedef struct {
    int16_t offset_y;
    float   table[SCREEN_HEIGHT/2];
} DepthToY_Lookup;

/** 
 * @brief where the renderer shall render a specific texture on top
*/
typedef struct {
    uint16_t num_rects;
    rect_t  *rects;
    color_t **pixels_by_rect;
    bool    *on_the_side;
    int16_t *layers; // layer -32768 behind everything, any negative layer indicate a depth of rendering
} LayeredTextures;

/**
 * shoots a single ray to figure out what is in this direction.
 */
HitInfo raycast_single_ray(Ray ray, Maze *maze);

/**
 * renders a frame by shoot many rays.
 */
void raycast_render(Player player, Maze *maze, Entity *entities, size_t num_entities, textures_t textures, LayeredTextures layers);

/**
 * creates an entity depth buffer where the closesed entity is stored for each slice
 * @note entities_depth MUST be of length SCREEN_WIDTH
 */
void get_entities_depth(Player player, EntityDepth *entities_depth, Entity *entities, size_t num_entities, textures_t textures);

#endif