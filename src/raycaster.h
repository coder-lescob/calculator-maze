#ifndef RAYCASTER_H
#define RAYCASTER_H

#include "vec.h"
#include "maze.h"
#include "entity.h"
#include "texture_loader.h"
#include "fixed_points.h"

/**
 * @brief this is the ray structure a point where the ray is shot and direction in which it has been
 */
typedef struct {
    Vec2 origine;
    Vec2 direction;
} Ray;

/// @brief a side is an uint8_t it would be wastful to store an uint32_t
typedef uint8_t Side;

#define UNDEFINED_SIDE 2
#define VERTICAL 0
#define HORIZONTAL 1

/**
 * @brief a structure to store all the infos about an hit
 */
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
    uint8_t num_entities; // I won't allow more than 255 entities to be viewed through
    uint8_t capacity;
    EntitySlice *entities;
} EntityDepth;

/**
 * @brief a lookup table to avoid doing too much divisons
*/
typedef struct {
    int16_t offset_y;
    fixed16_t table[SCREEN_HEIGHT/2];
} Y_ToDepthLookup;

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