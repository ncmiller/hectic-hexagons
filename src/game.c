#include "game_state.h"
#include "math.h"
#include <time.h>
#include <stdlib.h>
#include <assert.h>

#define ROTATION_TIME_MS 150
#define ROTATION_MAX_SCALE 1.5f

// Includes min and max
static int rand_in_range(int min, int max) {
    return min + (rand() % (max - min + 1));
}

static void spawn_hex(HexType type, int q, int r) {
    assert(q < HEX_NUM_COLUMNS && r < HEX_NUM_ROWS);
    Hex* hex = &g_state.hexes[q][r];

    // For even columns, the last row of hexes are not valid
    bool q_even = ((q & 1) == 0);
    if (q_even && (r == HEX_NUM_ROWS - 1)) {
        hex->is_valid = false;
        return;
    }

    const double s = HEX_WIDTH / 2;  // radius of hex
    const double h = sqrt(3) * s;    // height of flat-top hex
    const double w = 2 * s;          // width of flat-top hex

    const double board_width = 0.75f * w * HEX_NUM_COLUMNS;
    const double board_height = h * HEX_NUM_ROWS;

    const int board_x = LOGICAL_WINDOW_WIDTH / 2 - board_width / 2;
    const int board_y = LOGICAL_WINDOW_HEIGHT / 2 - board_height / 2;

    hex->hex_type = type;
    hex->scale = 1.0f;
    hex->alpha = 1.0f;

    // Compute screen space point of upper-left corner of hex texture
    hex->hex_point.x = board_x + (int)round(q * 0.75f * w);
    if (q_even) {
        hex->hex_point.y = board_y + (int)round(0.5f * h + r * h);
    } else {
        hex->hex_point.y = board_y + (int)round(r * h);
    }

    // Compute screen space point of rotation for 3-cluster rotations
    // TODO - maybe can use cursor point instead
    hex->rotation_point.x = board_x + (int)round(w);
    hex->rotation_point.y = board_y + (int)round(0.5f * h);
    hex->rotation_angle = 0.0f;

    hex->is_valid = true;
}

bool game_init(void) {
    srand(time(0));

    g_state.game.level = 1;
    g_state.game.combos_remaining = 50;
    g_state.game.score = 0;
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            spawn_hex(rand_in_range(0, NUM_HEX_TYPES - 1), q, r);
        }
    }
    return true;
}

void game_update(void) {
#if 0
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

            HexType temp1 = g_state.hexes[1].hex_type;
            HexType temp2 = g_state.hexes[2].hex_type;
            if (g_state.game.degrees_to_rotate > 0) {
                g_state.hexes[1].hex_type = g_state.hexes[0].hex_type;
                g_state.hexes[2].hex_type = temp1;
                g_state.hexes[0].hex_type = temp2;
            } else {
                g_state.hexes[1].hex_type = temp2;
                g_state.hexes[2].hex_type = g_state.hexes[0].hex_type;
                g_state.hexes[0].hex_type = temp1;
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
#endif
}
