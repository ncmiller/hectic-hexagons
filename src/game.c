#include "game_state.h"
#include "math.h"

bool game_init(void) {
    g_state.game.level = 1;
    g_state.game.combos_remaining = 50;
    g_state.game.score = 0;
    return true;
}

void game_update(void) {
    static bool animation_in_progress = false;
    static double degrees_to_rotate = 0.0f;
    static int animation_start = 0;
    const int animation_time_ms = 250;
    const double rotate_max_scale = 1.5f;
    // const int animation_time_ms = 1000;
    // const double rotate_max_scale = 2.0f;

    bool start_rotate_animation = g_state.input.rotate_cw || g_state.input.rotate_ccw;
    if (start_rotate_animation && !animation_in_progress) {
        animation_in_progress = true;
        if (g_state.input.rotate_cw) {
            degrees_to_rotate = 120.0f;
        } else if (g_state.input.rotate_ccw) {
            degrees_to_rotate = -120.0f;
        }
        animation_start = SDL_GetTicks();
    }

    if (animation_in_progress) {
        double animation_progress = (double)(SDL_GetTicks() - animation_start) / (double)animation_time_ms;
        if (animation_progress > 1.0f) {
            animation_in_progress = false;

            g_state.graphics.hexes[0].rotation_angle = 0.0f;
            g_state.graphics.hexes[1].rotation_angle = 0.0f;
            g_state.graphics.hexes[2].rotation_angle = 0.0f;

            g_state.graphics.hexes[0].scale = 1.0f;
            g_state.graphics.hexes[1].scale = 1.0f;
            g_state.graphics.hexes[2].scale = 1.0f;

            HexID temp1 = g_state.graphics.hexes[1].hex_id;
            HexID temp2 = g_state.graphics.hexes[2].hex_id;
            if (degrees_to_rotate > 0) {
                g_state.graphics.hexes[1].hex_id = g_state.graphics.hexes[0].hex_id;
                g_state.graphics.hexes[2].hex_id = temp1;
                g_state.graphics.hexes[0].hex_id = temp2;
            } else {
                g_state.graphics.hexes[1].hex_id = temp2;
                g_state.graphics.hexes[2].hex_id = g_state.graphics.hexes[0].hex_id;
                g_state.graphics.hexes[0].hex_id = temp1;
            }
        } else {
            double angle = animation_progress * degrees_to_rotate;
            g_state.graphics.hexes[0].rotation_angle = angle;
            g_state.graphics.hexes[1].rotation_angle = angle;
            g_state.graphics.hexes[2].rotation_angle = angle;

            double scale = 1.0f;
            if (animation_progress < 0.5f) {
                double s0 = 1.0f;
                double s1 = rotate_max_scale;
                double t = (animation_progress / 0.5f);
                scale = (1.0f - t) * s0 + t * s1;
            } else {
                double s0 = rotate_max_scale;
                double s1 = 1.0f;
                double t = ((animation_progress - 0.5f) / 0.5f);
                scale = (1.0f - t) * s0 + t * s1;
            }
            g_state.graphics.hexes[0].scale = scale;
            g_state.graphics.hexes[1].scale = scale;
            g_state.graphics.hexes[2].scale = scale;
        }
    }
}
