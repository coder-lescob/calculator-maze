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

#endif