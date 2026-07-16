#ifndef ENTITY_H
#define ENTITY_H

#include <eadk.h>
#include "vec.h"
#include "player.h"

/**
 * An entity like a monster, a collectable, ...
 */
typedef struct {
    uint8_t entity_type; // => textures, motion...
    Vec2 pos;            // current state position
} Entity;

/**
 * move an entity like it type says
 */
void entity_move(Entity *entity, float dt, Player *player);

#endif