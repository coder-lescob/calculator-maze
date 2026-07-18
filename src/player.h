#ifndef PLAYER_H
#define PLAYER_H

#include <eadk.h>
#include "vec.h"
#include "maze.h"
#include "input.h"

typedef struct {
    Vec2 pos;
    Vec2 dir;
    Vec2 plane;
} Player;

/**
 * creates a new player with these characteristics
 * @note field_of_view in radians
 */
Player new_player(Vec2 pos, Vec2 dir, float field_of_view);

/**
 * moves the player using calculator keyboard
 * @note dt in seconds
 */
void player_move(Player *player, keyboard_t *keyboard, float dt, Maze *maze);

/**
 * resoves collisions by modifying dx and dy to not enter in colision
 */
void resolve_collisons(Player *player, float *dx, float *dy, Maze *maze);

#endif