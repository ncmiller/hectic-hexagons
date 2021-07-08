#include "game_state.h"
#include "window.h"
#include "time_now.h"
#include "hex.h"
#include <stdlib.h>
#include <assert.h>

#define ROTATION_TIME_MS 150
#define ROTATION_MAX_SCALE 1.5f

// Convenience accessors to global state
static Game* game = &g_state.game;
static Input* input = &g_state.input;

static bool board_has_any_matches(void) {
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            if (hex_has_cluster_match(q, r)) {
                return true;
            }
            if (hex_has_flower_match(q, r)) {
                return true;
            }
        }
    }
    return false;
}

static void handle_input(void) {
    // Wait for rotation to finish before processing any more inputs
    if (game->rotation_in_progress) {
        return;
    }

    Cursor* cursor = &g_state.cursor;
    if (input->up) {
        input->up = false;
        cursor_up(cursor);
    } else if (input->down) {
        input->down = false;
        cursor_down(cursor);
    } else if (input->right) {
        input->right = false;
        cursor_right(cursor);
    } else if (input->left) {
        input->left = false;
        cursor_left(cursor);
    }

    bool start_rotation = input->rotate_cw || input->rotate_ccw;
    if (start_rotation && !game->rotation_in_progress) {
        game->rotation_in_progress = true;
        game->rotation_start_time = now_ms();

        // Determine if this is a starflower rotation
        HexType cursor_hex_type = g_state.hexes[cursor->hex_anchor.q][cursor->hex_anchor.r].type;
        bool is_starflower_rotation =
            (cursor->position == CURSOR_POS_ON) && (cursor_hex_type == HEX_TYPE_STARFLOWER);

        if (input->rotate_cw) {
            input->rotate_cw = false;
            game->degrees_to_rotate = (is_starflower_rotation ? 60.0f : 120.0f);
        } else if (input->rotate_ccw) {
            input->rotate_ccw = false;
            game->degrees_to_rotate = (is_starflower_rotation ? -60.0f : -120.0f);
        }
    }
}

// Only modifies the hex type during rotation
static void rotate_hexes(const HexCoord* coords, size_t num_coords, bool clockwise) {
    if (clockwise) {
        // Take the one at the end and put it at the beginning
        HexType end_hex_type = g_state.hexes[coords[num_coords - 1].q][coords[num_coords - 1].r].type;
        for (int i = num_coords - 2; i >= 0; i--) {
            const Hex* src = &g_state.hexes[coords[i].q][coords[i].r];
            Hex* dest = &g_state.hexes[coords[i+1].q][coords[i+1].r];
            dest->type = src->type;
        }
        g_state.hexes[coords[0].q][coords[0].r].type = end_hex_type;
    } else {
        // Take the one at the beginning and put it at the end
        HexType beginning_hex_type = g_state.hexes[coords[0].q][coords[0].r].type;
        for (int i = 0; i < num_coords - 1; i++) {
            const Hex* src = &g_state.hexes[coords[i+1].q][coords[i+1].r];
            Hex* dest = &g_state.hexes[coords[i].q][coords[i].r];
            dest->type = src->type;
        }
        g_state.hexes[coords[num_coords - 1].q][coords[num_coords - 1].r].type = beginning_hex_type;
    }
}

static void handle_rotation(void) {
    Cursor* cursor = &g_state.cursor;
    Hex* cursor_hex = &g_state.hexes[cursor->hex_anchor.q][cursor->hex_anchor.r];

    HexNeighbors neighbors = {0};
    cursor_neighbors(cursor, &neighbors);

    const double rotation_progress =
        (double)(now_ms() - game->rotation_start_time) / (double)ROTATION_TIME_MS;

    if (rotation_progress > 1.0f) {
        game->rotation_in_progress = false;

        cursor_hex->is_rotating = false;
        cursor_hex->rotation_angle = 0.0f;
        cursor_hex->scale = 1.0f;

        for (int i = 0; i < neighbors.num_neighbors; i++) {
            Hex* hex = &g_state.hexes[neighbors.coords[i].q][neighbors.coords[i].r];
            hex->is_rotating = false;
            hex->rotation_angle = 0.0f;
            hex->scale = 1.0f;
        }

        bool is_rotate_clockwise = (game->degrees_to_rotate > 0);
        if (cursor->position == CURSOR_POS_ON) {
            // starflower or black pearl rotation, rotate all neighbors
            rotate_hexes(neighbors.coords, neighbors.num_neighbors, is_rotate_clockwise);
        } else {
            // normal trio rotation, rotate the cursor and two neighbors
            HexCoord hexes_to_rotate[3];
            hexes_to_rotate[0] = cursor->hex_anchor;
            hexes_to_rotate[1] = neighbors.coords[0];
            hexes_to_rotate[2] = neighbors.coords[1];
            rotate_hexes(hexes_to_rotate, 3, is_rotate_clockwise);
        }
    } else {
        const double angle = rotation_progress * game->degrees_to_rotate;
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

        cursor_hex->is_rotating = true;
        cursor_hex->rotation_angle = angle;
        cursor_hex->scale = scale;

        for (int i = 0; i < neighbors.num_neighbors; i++) {
            Hex* hex = &g_state.hexes[neighbors.coords[i].q][neighbors.coords[i].r];
            hex->is_rotating = true;
            hex->rotation_angle = angle;
            hex->scale = scale;
        }
    }
}

bool game_init(void) {
    srand(now_ms());

    game->level = 1;
    game->combos_remaining = 50;
    game->score = 0;

    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            hex_spawn(q, r);
        }
    }

    int reroll_attempts = 0;
    while (board_has_any_matches()) {
        reroll_attempts++;
        assert(reroll_attempts < 100);

        // Fix cluster matches by rerolling (q,r)
        for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
            for (int r = 0; r < HEX_NUM_ROWS; r++) {
                if (hex_has_cluster_match(q, r)) {
                    g_state.hexes[q][r].type = hex_random_type();
                }
            }
        }

        // Fix starflower matches by rerolling top neighbor
        for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
            for (int r = 0; r < HEX_NUM_ROWS; r++) {
                if (hex_has_flower_match(q, r)) {
                    g_state.hexes[q][r-1].type = hex_random_type();
                }
            }
        }
    }

    cursor_init(&g_state.cursor);

    return true;
}

// The general plan for each game update is:
//
//  1. Get user input, possibly starting a rotation.
//  2. Update rotation and falling animations. If any rotation of fall is completed:
//      2a. Check for combos and score them.
//      2b. Remove combo'd hexes and respawn them above the board.
//      2c. Start falling animation for each hex above the combo'd hexes.
//
// Checking for combos is done in this order:
//  * Flowers
//  * Simple clusters (3, 4, or 5 of the same hex type, or multipliers clusters of any color)
//  * Bomb diffusals (if combined with a multiplier, this will eliminate all of that color)
//  * MMC clusters (whatever clusters remain, containing a mix of basic colors and multiplers)
//
// To find flowers, we simply iterate over the entire board, checking to see if each hex is the
// center of a starflower. It's dumb, but simple.
//
// To find clusters, we use iterative depth-first search.
void game_update(void) {
    handle_input();

    if (game->rotation_in_progress) {
        handle_rotation();
    }
}
