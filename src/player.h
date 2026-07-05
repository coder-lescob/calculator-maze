#ifndef PLAYER_H
#define PLAYER_H

#include <eadk.h>
#include "vec.h"
#include "maze.h"

typedef struct {
    Vec2 pos;

    // in radians
    float angle;
} Player;

/**
 * moves the player using calculator keyboard
 */
void player_move(Player *player, eadk_keyboard_state_t keyboard, float dt, Maze *maze);

/**
 * resoves collisions by modifying dx and dy to not enter in colision
 */
void resolve_collisons(Player *player, float *dx, float *dy, Maze *maze);

#endif