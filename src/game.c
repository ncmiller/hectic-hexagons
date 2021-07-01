#include "game_state.h"
#include "math.h"

#define ROTATION_TIME_MS 150
#define ROTATION_MAX_SCALE 1.5f

bool game_init(void) {
    g_state.game.level = 1;
    g_state.game.combos_remaining = 50;
    g_state.game.score = 0;
    return true;
}

void game_update(void) {
    bool start_rotation = g_state.input.rotate_cw || g_state.input.rotate_ccw;
    if (start_rotation && !g_state.game.rotation_in_progress) {
        g_state.game.rotation_in_progress = true;
        if (g_state.input.rotate_cw) {
            g_state.input.rotate_cw = false;
            g_state.game.degrees_to_rotate = 120.0f;
        } else if (g_state.input.rotate_ccw) {
            g_state.input.rotate_ccw = false;
            g_state.game.degrees_to_rotate = -120.0f;
        }
        g_state.game.rotation_start_time = SDL_GetTicks();
    }

    if (g_state.game.rotation_in_progress) {
        double rotation_progress =
            (double)(SDL_GetTicks() - g_state.game.rotation_start_time) / (double)ROTATION_TIME_MS;
        if (rotation_progress > 1.0f) {
            g_state.game.rotation_in_progress = false;

            g_state.hexes[0].rotation_angle = 0.0f;
            g_state.hexes[1].rotation_angle = 0.0f;
            g_state.hexes[2].rotation_angle = 0.0f;

            g_state.hexes[0].scale = 1.0f;
            g_state.hexes[1].scale = 1.0f;
            g_state.hexes[2].scale = 1.0f;

            HexID temp1 = g_state.hexes[1].hex_id;
            HexID temp2 = g_state.hexes[2].hex_id;
            if (g_state.game.degrees_to_rotate > 0) {
                g_state.hexes[1].hex_id = g_state.hexes[0].hex_id;
                g_state.hexes[2].hex_id = temp1;
                g_state.hexes[0].hex_id = temp2;
            } else {
                g_state.hexes[1].hex_id = temp2;
                g_state.hexes[2].hex_id = g_state.hexes[0].hex_id;
                g_state.hexes[0].hex_id = temp1;
            }
        } else {
            double angle = rotation_progress * g_state.game.degrees_to_rotate;
            g_state.hexes[0].rotation_angle = angle;
            g_state.hexes[1].rotation_angle = angle;
            g_state.hexes[2].rotation_angle = angle;

            double scale = 1.0f;
            if (rotation_progress < 0.5f) {
                double s0 = 1.0f;
                double s1 = ROTATION_MAX_SCALE;
                double t = (rotation_progress / 0.5f);
                scale = (1.0f - t) * s0 + t * s1;
            } else {
                double s0 = ROTATION_MAX_SCALE;
                double s1 = 1.0f;
                double t = ((rotation_progress - 0.5f) / 0.5f);
                scale = (1.0f - t) * s0 + t * s1;
            }
            g_state.hexes[0].scale = scale;
            g_state.hexes[1].scale = scale;
            g_state.hexes[2].scale = scale;
        }
    }
}
