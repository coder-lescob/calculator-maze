#ifndef RAYCASTER_H
#define RAYCASTER_H

#include "vec.h"
#include "maze.h"

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
    uint8_t block_type;
    Side side;
} HitInfo;

/**
 * shoots a single ray to figure out what is in this direction.
 */
HitInfo raycast_single_ray(Ray ray, Maze *maze);

/**
 * renders a frame by shoot many rays.
 */
void raycast_render(Vec2 pos, Maze *maze, float player_angle);

#endif