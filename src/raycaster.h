#ifndef RAYCASTER_H
#define RAYCASTER_H

#include "vec.h"
#include "maze.h"

typedef struct {
    Vec2 origine;
    Vec2 direction;
} Ray;

uint16_t get_tile_idx(Vec2 pos, Maze *maze);

/**
 * (0, 0) as a pos would be at the left top corner of the maze 
 */
float raycast_for_distance(Ray ray, Maze *maze);

void raycast_render(Vec2 pos, Maze *maze, float player_angle);

#endif