#ifndef MAZE_H
#define MAZE_H

#include <eadk.h>
#include <stdint.h>

/**
 * all tiles are 1x1
 */
typedef struct {
    uint16_t width, height;
    uint8_t *tiles;
} Maze;

/**
 * generates a maze as wide as width and as high as height
 * @note You MUST call free maze on this maze.
 */
Maze generate_maze(uint16_t width, uint16_t height);

/**
 * free a maze
 */
void free_maze(Maze *maze);

/**
 * prints a maze to the console (use for debugging)
 */
void print_maze(Maze *maze);


#endif