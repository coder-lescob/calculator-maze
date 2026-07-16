#ifndef RAYCASTER_H
#define RAYCASTER_H

#include "vec.h"
#include "maze.h"
#include "entity.h"

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
} HitInfo;

/**
 * to sort entities
 */
struct EntityEntry {
    uint16_t index;
    float dst;
};

/**
 * shoots a single ray to figure out what is in this direction.
 */
HitInfo raycast_single_ray(Ray ray, Maze *maze);

/**
 * renders a frame by shoot many rays.
 */
void raycast_render(Player player, Maze *maze, Entity *entities, size_t num_entities);

/**
 * try to draw the entities if the distance from the enity to the player is in between distance and next_distance it renders else it does not.
 */
void draw_entities(Player player, float *depth_buffer, Entity *entities, size_t num_entities);

/**
 * use quick sort to sort all the entities by distance.
 * @note entity order MUST have the same length as entities
 */
void sort_entities_by_distance(Player player, size_t num_entities, Entity *entities, uint16_t *entity_order);

#endif