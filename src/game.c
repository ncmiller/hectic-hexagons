#include "game_state.h"
#include "window.h"
#include "time_now.h"
#include "hex.h"
#include "macros.h"
#include "bump_allocator.h"
#include "text.h"
#include "test_boards.h"
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

#define ROTATION_TIME_MS 150
#define ROTATION_MAX_SCALE 1.5f
#define FLOWER_MATCH_ANIMATION_TIME_MS 800
#define FLOWER_MATCH_MAX_SCALE 1.7f
#define LOCAL_SCORE_ANIMATION_TIME_MS 1200
#define LOCAL_SCORE_ANIMATION_MAX_HEIGHT HEX_HEIGHT

// Convenience accessors to global state
static Game* game = &g_state.game;

static bool board_has_any_matches(void) {
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            if (hex_has_cluster_match(q, r, NULL, NULL)) {
                return true;
            }
            if (hex_has_flower_match(q, r)) {
                return true;
            }
        }
    }
    return false;
}

static void hex_coord_print(const void* vector_item, char* buffer, size_t buffer_size) {
    HexCoord coord = *(const HexCoord*)vector_item;
    snprintf(buffer, buffer_size, "(q, r) = (%d, %d)", coord.q, coord.r);
}

static void flower_animation_print(const void* vector_item, char* buffer, size_t buffer_size) {
    const FlowerMatchAnimation* fma = (const FlowerMatchAnimation*)vector_item;
    snprintf(buffer, buffer_size,
        "in_progress = %u, start_time = %" PRIu64 ", flower_center = (%u,%u)",
        fma->in_progress,
        fma->start_time,
        fma->flower_center.q,
        fma->flower_center.r);
}

static bool flower_animation_in_progress(const void* vector_item) {
    const FlowerMatchAnimation* fma = (const FlowerMatchAnimation*)vector_item;
    return !fma->in_progress;
}

static bool local_score_animation_in_progress(const void* vector_item) {
    const LocalScoreAnimation* lsa = (const LocalScoreAnimation*)vector_item;
    return !lsa->in_progress;
}

static void handle_flower_match_animations(void) {
    for (size_t i = 0; i < vector_size(game->flower_match_animations); i++) {
        FlowerMatchAnimation* fma = (FlowerMatchAnimation*)
            vector_data_at(game->flower_match_animations, i);

        const double animation_progress =
            (double)(now_ms() - fma->start_time) /
            (double)FLOWER_MATCH_ANIMATION_TIME_MS;

        HexNeighbors neighbors = {0};
        HexCoord coord = fma->flower_center;
        hex_neighbors(coord.q, coord.r, &neighbors, ALL_NEIGHBORS);

        if (animation_progress > 1.0f) {
            fma->in_progress = false;
            for (int i = 0; i < neighbors.num_neighbors; i++) {
                Hex* hex = &g_state.hexes[neighbors.coords[i].q][neighbors.coords[i].r];
                hex->is_flower_fading = false;
                hex->alpha = 1.0f;
                hex->scale = 1.0f;
            }
        } else {
            double alpha = 0.0f;
            double scale = 0.0f;

            { // compute alpha
                double s0 = 1.0f;
                double s1 = 0.0f;
                double t = animation_progress;
                t = t * t; // ease in, smash out
                alpha = (1.0f - t) * s0 + t * s1;
            }

            { // compute scale
                double s0 = 1.0f;
                double s1 = FLOWER_MATCH_MAX_SCALE;
                double t = animation_progress;
                scale = (1.0f - t) * s0 + t * s1;
            }

            for (int i = 0; i < neighbors.num_neighbors; i++) {
                Hex* hex = &g_state.hexes[neighbors.coords[i].q][neighbors.coords[i].r];
                hex->is_flower_fading = true;
                hex->flower_center = fma->flower_center;
                hex->alpha = alpha;
                hex->scale = scale;
            }
        }
    }

    // Erase completed animations
    // vector_print(game->flower_match_animations, flower_animation_print);
    vector_erase_if(game->flower_match_animations, flower_animation_in_progress);
}

static void handle_local_score_animations(void) {
    for (size_t i = 0; i < vector_size(game->local_score_animations); i++) {
        LocalScoreAnimation* lsa = (LocalScoreAnimation*)
            vector_data_at(game->local_score_animations, i);

        const double animation_progress =
            (double)(now_ms() - lsa->start_time) /
            (double)LOCAL_SCORE_ANIMATION_TIME_MS;

        if (animation_progress > 1.0f) {
            lsa->in_progress = false;
        } else {
            double alpha = 0.0f;
            double height_delta = 0.0f;

            { // alpha
                double s0 = 1.0f;
                double s1 = 0.1f;
                double t = animation_progress;
                t = t * t; // ease in, smash out
                alpha = (1.0f - t) * s0 + t * s1;
            }

            { // height_delta
                double s0 = 0.0f;
                double s1 = LOCAL_SCORE_ANIMATION_MAX_HEIGHT;
                double t = animation_progress;
                height_delta = (1.0f - t) * s0 + t * s1;
            }

            lsa->alpha = alpha;
            lsa->current_point.y = lsa->start_point.y - (int)height_delta;
        }
    }

    // Erase completed animations
    vector_erase_if(game->local_score_animations, local_score_animation_in_progress);
}

static void handle_input(void) {
    bool go_up = false;
    bool go_down = false;
    bool go_left = false;
    bool go_right = false;
    bool rotate_cw = false;
    bool rotate_ccw = false;

    Input* input = &g_state.input;
    Cursor* cursor = &g_state.cursor;
    if (input->up) {
        input->up = false;
        go_up = true;
    }
    if (input->down) {
        input->down = false;
        go_down = true;
    }
    if (input->right) {
        input->right = false;
        go_right = true;
    }
    if (input->left) {
        input->left = false;
        go_left = true;
    }
    if (input->spacebar) {
        input->spacebar = false;
        test_boards_print_current();
    }
    if (input->rotate_cw) {
        input->rotate_cw = false;
        rotate_cw = true;
    }
    if (input->rotate_ccw) {
        input->rotate_ccw = false;
        rotate_ccw = true;
    }

    bool animation_in_progress =
        (vector_size(game->flower_match_animations) > 0) ||
        game->rotation_in_progress;

    if (animation_in_progress) {
        return;
    }

    if (go_up) {
        cursor_up(cursor);
    }
    if (go_down) {
        cursor_down(cursor);
    }
    if (go_left) {
        cursor_left(cursor);
    }
    if (go_right) {
        cursor_right(cursor);
    }

    bool start_rotation = rotate_cw || rotate_ccw;
    if (start_rotation && !game->rotation_in_progress) {
        game->rotation_in_progress = true;
        game->rotation_start_time = now_ms();

        // Determine if this is a starflower rotation
        HexType cursor_hex_type = g_state.hexes[cursor->hex_anchor.q][cursor->hex_anchor.r].type;
        bool is_starflower_rotation =
            (cursor->position == CURSOR_POS_ON) && (cursor_hex_type == HEX_TYPE_STARFLOWER);

        if (rotate_cw) {
            game->degrees_to_rotate = (is_starflower_rotation ? 60.0f : 120.0f);
        } else if (rotate_ccw) {
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

// Returns true if rotation was completed.
static bool handle_rotation(void) {
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
        return true;
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
        return false;
    }
}

// Returns true if any hexes became newly locked (were falling, but no longer falling)
bool handle_physics(void) {
    bool hex_became_locked = false;
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            Hex* hex = hex_at(q, r);
            if (hex->is_falling) {
                // TODO - update falling animation, detect collisions
                bool collided = false;
                if (collided) {
                    // TODO - set final position
                    hex->is_falling = false;
                    hex_became_locked = true;
                }
            }
        }
    }

    return hex_became_locked;
}

static void handle_simple_cluster(const HexCoord* hex_coords, size_t num_coords) {
    assert(num_coords >= 3 && num_coords <= 5);

    if (game->combos_remaining > 0) {
        game->combos_remaining--;
    }

    for (size_t i = 0; i < num_coords; i++) {
        HexCoord c = hex_coords[i];
        Hex* hex = hex_at(c.q, c.r);
        hex->is_matched = true;
    }

    // TODO - compute local score
    uint32_t local_score = 100;

    // TODO - start cluster match animation

    Rectangle r = hex_bounding_box_of_coords(hex_coords, num_coords);
    Point cluster_center = {
        .x = r.top_left.x + r.width / 2,
        .y = r.top_left.y + r.height / 2,
    };

    // Start local score animation
    LocalScoreAnimation lsa = {
        .in_progress = true,
        .start_time = now_ms(),
        .score = (uint32_t)local_score,
        .alpha = 1.0f,
        .start_point = cluster_center,
        .current_point = cluster_center,
    };
    text_init(&lsa.text);
    vector_push_back(game->local_score_animations, &lsa);
}

// Computes score, updates combos remaining, and marks hexes as matched.
static void handle_flower(const HexCoord* hex_coords, size_t num_coords) {
    assert(num_coords == 7);

    if (game->combos_remaining > 0) {
        game->combos_remaining--;
    }

    Hex* center_hex = &g_state.hexes[hex_coords[0].q][hex_coords[0].r];
    bool starflower_center = hex_is_starflower(center_hex);
    bool black_pearl_center = hex_is_black_pearl(center_hex);

    center_hex->is_matched = true;

    size_t num_neighbor_multipliers = 0;
    bool all_neighbors_starflower = true;
    bool all_neighbors_black_pearl = true;
    for (size_t i = 0; i < 6; i++) {
        Hex* n_hex = &g_state.hexes[hex_coords[i+1].q][hex_coords[i+1].r];

        num_neighbor_multipliers += (hex_is_multiplier(n_hex) ? 1 : 0);
        if (!hex_is_starflower(n_hex)) {
            all_neighbors_starflower = false;
        }
        if (!hex_is_black_pearl(n_hex)) {
            all_neighbors_black_pearl = false;
        }

        n_hex->is_matched = true;
    }

    if (all_neighbors_starflower) {
        uint32_t mask = (1 << HEX_TYPE_BLACK_PEARL_UP) | (1 << HEX_TYPE_BLACK_PEARL_DOWN);
        center_hex->type = hex_random_type_with_mask(mask);
    } else if (all_neighbors_black_pearl) {
        // TODO - set flag to end game
    } else {
        center_hex->type = HEX_TYPE_STARFLOWER;
    }

    // Base score
    double local_score = game->level * 1000.0f;

    // Multiplier bonus for center hex
    if (starflower_center) {
        local_score *= 1.5f;
    } else if (black_pearl_center) {
        local_score *= 2.5f;
    }

    // Multiplier bonus for neighbor hexes
    if (all_neighbors_starflower) {
        local_score *= 10.0f;
    } else if (all_neighbors_black_pearl) {
        local_score *= 200.0f;
    } else {
        for (int i = 0; i < num_neighbor_multipliers; i++) {
            local_score *= 2.0f;
        }
    }

    game->score += (uint32_t)local_score;

    // Start flower match animation
    FlowerMatchAnimation fma = {
        .in_progress = true,
        .start_time = now_ms(),
        .flower_center = hex_coords[0],
    };
    vector_push_back(game->flower_match_animations, &fma);

    // Start local score animation
    Point start_point = (Point){
        .x = center_hex->hex_point.x + HEX_WIDTH / 2,
        .y = center_hex->hex_point.y + HEX_HEIGHT / 2,
    };
    LocalScoreAnimation lsa = {
        .in_progress = true,
        .start_time = now_ms(),
        .score = (uint32_t)local_score,
        .alpha = 1.0f,
        .start_point = start_point,
        .current_point = start_point,
    };
    text_init(&lsa.text);
    vector_push_back(game->local_score_animations, &lsa);
}

// The general plan for each game update is:
//
//  1. Get user input, possibly starting a rotation.
//  2. Update rotation and falling animations. If any rotation or fall is completed:
//      2a. Check for combos and score them.
//      2b. Remove combo'd hexes and respawn them above the board.
//      2c. Start falling animation for each hex above the combo'd hexes.
//
// Checking for combos is done in this order:
//  * Flowers
//  * Simple clusters (3, 4, or 5 of the same hex type, or multipliers clusters of any color)
//  * Bomb diffusals (if combined with a multiplier, this will eliminate all of that color)
//  * MMC clusters (whatever clusters remain, containing a mix of basic colors and multiplers)
void game_update(void) {
    handle_input();

    bool rotation_finished = false;
    if (game->rotation_in_progress) {
        rotation_finished = handle_rotation();
    }

    if (vector_size(game->flower_match_animations) > 0) {
        handle_flower_match_animations();
    }

    if (vector_size(game->local_score_animations) > 0) {
        handle_local_score_animations();
    }

    bool hex_finished_falling = handle_physics();

    if (rotation_finished || hex_finished_falling) {
        // Flowers
        Vector flower = vector_create_with_allocator(
                sizeof(HexCoord), bump_allocator_alloc, bump_allocator_free);
        vector_reserve(flower, 7);
        size_t iteration = 0;
        while (1) {
            vector_clear(flower);
            size_t flower_size = hex_find_one_flower(flower);
            if (flower_size == 0) {
                break;
            }
            handle_flower(vector_data_at(flower, 0), vector_size(flower));
            assert(iteration++ < 100);
        }

        // Simple clusters
        Vector simple_cluster = vector_create_with_allocator(
                sizeof(HexCoord), bump_allocator_alloc, bump_allocator_free);
        vector_reserve(simple_cluster, 5);
        iteration = 0;
        while (1) {
            vector_clear(simple_cluster);
            size_t simple_cluster_size = hex_find_one_simple_cluster(simple_cluster);
            if (simple_cluster_size == 0) {
                break;
            }
            vector_print(simple_cluster, hex_coord_print);
            handle_simple_cluster(vector_data_at(simple_cluster, 0), vector_size(simple_cluster));
            assert(iteration++ < 100);
        }

        // TODO - check for bomb diffusals
        // TODO - check for MMCs
        // TODO - trigger hexes to fall
        // TODO - respawn cleared hexes

        hex_for_each(hex_clear_is_matched);
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
                if (hex_has_cluster_match(q, r, NULL, NULL)) {
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

    // For testing - load a specific board
    test_boards_load(g_test_board_six_black_pearls);

    cursor_init(&g_state.cursor);

    game->local_score_animations = vector_create(sizeof(LocalScoreAnimation));
    game->flower_match_animations = vector_create(sizeof(FlowerMatchAnimation));

    vector_reserve(game->local_score_animations, 10);
    vector_reserve(game->flower_match_animations, 10);

    return true;
}
