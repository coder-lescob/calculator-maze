#include "player.h"
#include <math.h>
#include <stdio.h>

// Yes I actually know that much
#define PI 3.141592653589793f

static void fix_player_angle(Player *player) {
    // clamp the angle in [0; 2*PI]
    if (player->angle < 0) {
        player->angle = 2 * PI;
    }
    if (player->angle > 2 * PI) {
        player->angle = 0;
    }
}

void player_move(Player *player, eadk_keyboard_state_t keyboard, float dt, Maze *maze) {
    // rotate the player
    player->angle += PI * (eadk_keyboard_key_down(keyboard, eadk_key_right) - eadk_keyboard_key_down(keyboard, eadk_key_left)) * dt;
    fix_player_angle(player);

    // move him/her in the direction of looking
    float forward_movement = (eadk_keyboard_key_down(keyboard, eadk_key_up) - eadk_keyboard_key_down(keyboard, eadk_key_down)) * dt;
    float dx = cosf(player->angle) * forward_movement;
    float dy = sinf(player->angle) * forward_movement;

    // resolves collisions
    resolve_collisons(player, &dx, &dy, maze);

    // apply movement
    player->pos = vec_add(player->pos, (Vec2) { dx, dy });
}

#define HITBOX_SIZE (0.1f)

static bool point_collide(Vec2 p, Maze *maze) {
    // is point outside
    if (p.x < 0 || p.y < 0 || p.x >= maze->width || p.y >= maze->height) {
        return true;
    }

    // does the point touche a solide tile
    int index = (int)p.x + (int)p.y * maze->width;
    return maze->tiles[index] > 0;
}

static bool is_collision_found(Vec2 pos, Maze *maze) {

    // loop along the x axis to check collision along the top and bottom segments of the player's hitbox
    for (float x = pos.x - HITBOX_SIZE; x <= pos.x + HITBOX_SIZE; x += HITBOX_SIZE / 10.0f) {
        float y0 = pos.y + HITBOX_SIZE;
        float y1 = pos.y - HITBOX_SIZE;

        if (point_collide((Vec2) { x, y0 }, maze) || point_collide((Vec2) { x, y1 }, maze)) {
            return true;
        }
    }

    // loop along the y axis to check collision along the left and rights segments of the player's hitbox
    for (float y = pos.y - HITBOX_SIZE; y <= pos.y + HITBOX_SIZE; y += HITBOX_SIZE / 10.0f) {
        float x0 = pos.x + HITBOX_SIZE;
        float x1 = pos.x - HITBOX_SIZE;
        
        if (point_collide((Vec2) { x0, y }, maze) || point_collide((Vec2) { x1, y }, maze)) {
            return true;
        }
    }

    // no collision detected
    return false;
}

void resolve_collisons(Player *player, float *dx, float *dy, Maze *maze) {
    if (is_collision_found(player->pos, maze)) {
        return;
    }

    // if after moving a collision is found then don't allow moving there
    if (is_collision_found(vec_add(player->pos, (Vec2) { *dx, 0.0f }), maze)) {
        *dx = 0;
    }

    // same for dy
    if (is_collision_found(vec_add(player->pos, (Vec2) { 0.0f, *dy }), maze)) {
        *dy = 0;
    }
}