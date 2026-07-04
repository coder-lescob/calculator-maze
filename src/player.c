#include "player.h"
#include <math.h>

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

void player_move(Player *player, eadk_keyboard_state_t keyboard, float dt) {
    // rotate the player
    player->angle += PI * (eadk_keyboard_key_down(keyboard, eadk_key_right) - eadk_keyboard_key_down(keyboard, eadk_key_left)) * dt;
    fix_player_angle(player);

    // move him/her in the direction of looking
    float forward_movement = (eadk_keyboard_key_down(keyboard, eadk_key_up) - eadk_keyboard_key_down(keyboard, eadk_key_down)) * dt;
    float dx = cosf(player->angle) * forward_movement;
    float dy = sinf(player->angle) * forward_movement;

    // resolves collisions
    resolve_collisons(player, &dx, &dy);

    // apply movement
    player->pos = vec_add(player->pos, (Vec2) { dx, dy });
}

void resolve_collisons(Player *player, float *dx, float *dy) {
    /**
     * TODO: actually resolve collisions
     */
}