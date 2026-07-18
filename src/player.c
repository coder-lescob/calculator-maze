#include "player.h"
#include <math.h>
#include <stdio.h>

// Yes I actually know that much
#define PI 3.141592653589793f

Player new_player(Vec2 pos, Vec2 dir, float field_of_view) {
    // useful to scale the plane vector
    float tan_half_fov = tan(field_of_view / 2);

    return (Player) {
        .pos = pos,
        .dir = dir,
        .plane = (Vec2) { -dir.y * tan_half_fov, dir.x * tan_half_fov },
    };
}

static void rotate_player_by_angle(Player *player, float d_angle) {
    // roation matrix from https://en.wikipedia.org/wiki/Rotation_matrix
    // rotate the player direction and plane by d_angle
    float sin_dangle = sinf(d_angle), cos_dangle = cosf(d_angle);

    // [ cos d_angle    -sin d_angle ]
    // [                             ] * dir => rotates dir by d_angle
    // [ sin d_angle     cos d_angle ]

    // rotate dir
    float tmp = player->dir.x;
    player->dir.x = player->dir.x * cos_dangle - player->dir.y * sin_dangle;
    player->dir.y = tmp * sin_dangle + player->dir.y * cos_dangle;

    // rotate plane
    tmp = player->plane.x;
    player->plane.x = player->plane.x * cos_dangle - player->plane.y * sin_dangle;
    player->plane.y = tmp * sin_dangle + player->plane.y * cos_dangle;
}

void player_move(Player *player, keyboard_t *keyboard, float dt, Maze *maze) {
    // rotate the player
    float d_angle = PI * (key_pressed(keyboard, key_right) - key_pressed(keyboard, key_left)) * dt;
    rotate_player_by_angle(player, d_angle);

    // move him/her in the direction of looking
    float forward_movement = (key_pressed(keyboard, key_up) - key_pressed(keyboard, key_down)) * dt;
    float dx = player->dir.x * forward_movement;
    float dy = player->dir.y * forward_movement;

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