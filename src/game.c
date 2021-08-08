#include "game_state.h"
#include "window.h"
#include "hex.h"
#include "macros.h"
#include "time_utils.h"
#include "bump_allocator.h"
#include "text.h"
#include "test_boards.h"
#include "macros.h"
#include "audio.h"
#include <stdlib.h>
#include <inttypes.h>

#define ROTATION_TIME_MS 150
#define ROTATION_MAX_SCALE 1.5f
#define FLOWER_MATCH_ANIMATION_TIME_MS 800
#define FLOWER_MATCH_MAX_SCALE 1.7f
#define LOCAL_SCORE_ANIMATION_TIME_MS 1200
#define LOCAL_SCORE_ANIMATION_MAX_HEIGHT HEX_HEIGHT
#define CLUSTER_MATCH_ANIMATION_TIME_MS 500
#define GRAVITY_INITIAL 10.0f
#define GRAVITY_NORMAL 0.5f
#define MAX_VELOCITY 50
#define HEX_GRAVITY_DELAY_MS 200

// Convenience accessors to global state
static Game* game = &g_state.game;

static bool board_has_any_matches(bool require_stationary) {
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            if (hex_has_cluster_match(q, r, NULL, NULL, require_stationary)) {
                return true;
            }
            if (hex_has_flower_match(q, r, require_stationary)) {
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

static bool local_score_animation_in_progress(const void* vector_item) {
    const LocalScoreAnimation* lsa = (const LocalScoreAnimation*)vector_item;
    return !lsa->in_progress;
}

static bool hex_is_dead(const void* vector_item) {
    const Hex* hex = (const Hex*)vector_item;
    return hex->is_dead;
}

static void handle_flower_match_animations(void) {
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        int respawn_count = 0;
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            Hex* hex = hex_at(q, r);
            FlowerMatchAnimation* fma = &hex->flower_match_animation;
            if (!fma->in_progress) {
                continue;
            }

            const double animation_progress =
                (double)(g_state.frame_count - fma->start_time) /
                (double)ms_to_frames(FLOWER_MATCH_ANIMATION_TIME_MS);

            if (animation_progress > 1.0f) {
                fma->in_progress = false;
                hex->is_matched = false;
                hex->is_flower_matched = false;
                if (!fma->is_center) {
                    // Respawn the perimeter of the flower
                    hex->is_dead = true;
                    respawn_count++;
                }
            } else {
                if (!fma->is_center) {
                    { // compute alpha
                        double s0 = 1.0f;
                        double s1 = 0.0f;
                        double t = animation_progress;
                        t = t * t; // ease in, smash out
                        hex->alpha = (1.0f - t) * s0 + t * s1;
                    }

                    { // compute scale
                        double s0 = 1.0f;
                        double s1 = FLOWER_MATCH_MAX_SCALE;
                        double t = animation_progress;
                        hex->scale = (1.0f - t) * s0 + t * s1;
                    }
                }
            }
        }

        vector_erase_if(g_state.game.hexes[q], hex_is_dead);

        // Respawn dead hexes
        uint32_t now = g_state.frame_count;
        for (int i = 0; i < respawn_count; i++) {
            const Hex* stack_top = hex_at(q, hex_stack_index_to_row(vector_size(g_state.game.hexes[q]) - 1));
            Hex* new_hex = hex_spawn(q);

            // Start gravity after a short delay, making sure to start gravity
            // after the hex below.
            uint32_t gravity_start_base = (i == 0) ?
                MAX(now, stack_top->gravity_start_time) :
                stack_top->gravity_start_time;
            new_hex->gravity_start_time = gravity_start_base + ms_to_frames(HEX_GRAVITY_DELAY_MS);
        }
    }
}

static void handle_cluster_match_animations(void) {
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        int respawn_count = 0;
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            Hex* hex = hex_at(q, r);
            ClusterMatchAnimation* cma = &hex->cluster_match_animation;
            if (!cma->in_progress) {
                continue;
            }

            const double animation_progress =
                (double)(g_state.frame_count - cma->start_time) /
                (double)ms_to_frames(CLUSTER_MATCH_ANIMATION_TIME_MS);

            if (animation_progress > 1.0f) {
                cma->in_progress = false;
                hex->is_dead = true;
                respawn_count++;
            } else {
                double s0 = 1.0f;
                double s1 = 0.0f;
                double t = animation_progress;
                hex->scale = (1.0f - t) * s0 + t * s1;
            }
        }

        vector_erase_if(g_state.game.hexes[q], hex_is_dead);

        // Respawn dead hexes
        uint32_t now = g_state.frame_count;
        for (int i = 0; i < respawn_count; i++) {
            const Hex* stack_top = hex_at(q, hex_stack_index_to_row(vector_size(g_state.game.hexes[q]) - 1));
            Hex* new_hex = hex_spawn(q);

            // Start gravity after a short delay, making sure to start gravity
            // after the hex below.
            uint32_t gravity_start_base = (i == 0) ?
                MAX(now, stack_top->gravity_start_time) :
                stack_top->gravity_start_time;
            new_hex->gravity_start_time = gravity_start_base + ms_to_frames(HEX_GRAVITY_DELAY_MS);
        }
    }
}

static void handle_local_score_animations(void) {
    for (size_t i = 0; i < vector_size(game->local_score_animations); i++) {
        LocalScoreAnimation* lsa = (LocalScoreAnimation*)
            vector_data_at(game->local_score_animations, i);

        const double animation_progress =
            (double)(g_state.frame_count - lsa->start_time) /
            (double)ms_to_frames(LOCAL_SCORE_ANIMATION_TIME_MS);

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
    if (input->print_board) {
        input->print_board = false;
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

    // Entire board needs to be stationary before we act on user input
    if (!hex_all_stationary_no_animation()) {
        return;
    }

    bool cursor_moved = false;
    if (go_up) {
        cursor_moved |= cursor_up(cursor);
    }
    if (go_down) {
        cursor_moved |= cursor_down(cursor);
    }
    if (go_left) {
        cursor_moved |= cursor_left(cursor);
    }
    if (go_right) {
        cursor_moved |= cursor_right(cursor);
    }

    if (cursor_moved) {
        audio_play_sound_effect(AUDIO_MOVE_CURSOR);
    }


    bool start_rotation = rotate_cw || rotate_ccw;
    if (start_rotation && !game->rotation_animation.in_progress) {
        // Determine how many degrees to rotate based on hex type
        HexType cursor_hex_type = hex_at(cursor->hex_anchor.q, cursor->hex_anchor.r)->type;
        bool is_starflower_rotation =
            (cursor->position == CURSOR_POS_ON) && (cursor_hex_type == HEX_TYPE_STARFLOWER);
        bool is_blackflower_rotation =
            (cursor->position == CURSOR_POS_ON) && (cursor_hex_type == HEX_TYPE_STARFLOWER);
        bool is_trio_rotation = !(is_starflower_rotation || is_blackflower_rotation);
        double degrees_to_rotate = (is_starflower_rotation ? 60.0f : 120.0f);
        if (rotate_ccw) {
            degrees_to_rotate *= -1.0f;
        }

        game->rotation_animation = (RotationAnimation){
            .in_progress = true,
            .start_time = g_state.frame_count,
            .is_trio_rotation = is_trio_rotation,
            .rotation_center = g_state.cursor.screen_point,
            .degrees_to_rotate = degrees_to_rotate,
            .rotation_count = 0,
        };
    }
}

// Only modifies the hex type during rotation
static void rotate_hexes(const HexCoord* coords, size_t num_coords, bool clockwise) {
    if (clockwise) {
        // Take the one at the end and put it at the beginning
        HexType end_hex_type = hex_at(coords[num_coords - 1].q, coords[num_coords - 1].r)->type;
        for (int i = num_coords - 2; i >= 0; i--) {
            const Hex* src = hex_at(coords[i].q, coords[i].r);
            Hex* dest = hex_at(coords[i+1].q, coords[i+1].r);
            dest->type = src->type;
        }
        hex_at(coords[0].q, coords[0].r)->type = end_hex_type;
    } else {
        // Take the one at the beginning and put it at the end
        HexType beginning_hex_type = hex_at(coords[0].q, coords[0].r)->type;
        for (int i = 0; i < num_coords - 1; i++) {
            const Hex* src = hex_at(coords[i+1].q, coords[i+1].r);
            Hex* dest = hex_at(coords[i].q, coords[i].r);
            dest->type = src->type;
        }
        hex_at(coords[num_coords - 1].q, coords[num_coords - 1].r)->type = beginning_hex_type;
    }
}

// Returns true if rotation was completed.
static void handle_rotation(void) {
    if (!game->rotation_animation.in_progress) {
        return;
    }

    Cursor* cursor = &g_state.cursor;
    Hex* cursor_hex = hex_at(cursor->hex_anchor.q, cursor->hex_anchor.r);

    HexNeighbors neighbors = {0};
    cursor_neighbors(cursor, &neighbors);

    const double rotation_progress =
        (double)((double)g_state.frame_count - (double)game->rotation_animation.start_time) /
        (double)ms_to_frames(ROTATION_TIME_MS);

    // Rotation progress might be negative if we are stalling between automatic trio rotations.
    if (rotation_progress < 0.0f) {
        return;
    }

    if (rotation_progress > 1.0f) {
        cursor_hex->rotation_angle = 0.0f;
        cursor_hex->scale = 1.0f;

        for (int i = 0; i < neighbors.num_neighbors; i++) {
            Hex* hex = hex_at(neighbors.coords[i].q, neighbors.coords[i].r);
            hex->rotation_angle = 0.0f;
            hex->scale = 1.0f;
        }

        bool is_rotate_clockwise = (game->rotation_animation.degrees_to_rotate > 0);
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

        game->rotation_animation.rotation_count++;
        if (game->rotation_animation.is_trio_rotation &&
            (game->rotation_animation.rotation_count < 3) &&
            !board_has_any_matches(true)) {
            // Start another rotation in 100 ms
            game->rotation_animation.start_time = g_state.frame_count + ms_to_frames(100);
        } else {
            game->rotation_animation.in_progress = false;
            cursor_hex->is_rotating = false;
            for (int i = 0; i < neighbors.num_neighbors; i++) {
                hex_at(neighbors.coords[i].q, neighbors.coords[i].r)->is_rotating = false;
            }
        }
    } else {
        if (rotation_progress == 0.0f) {
            // TODO - play rotation sound effect
        }

        const double angle = rotation_progress * game->rotation_animation.degrees_to_rotate;
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

        cursor_hex->rotation_angle = angle;
        cursor_hex->scale = scale;
        cursor_hex->is_rotating = true;

        for (int i = 0; i < neighbors.num_neighbors; i++) {
            Hex* hex = hex_at(neighbors.coords[i].q, neighbors.coords[i].r);
            hex->is_rotating = true;
            hex->rotation_angle = angle;
            hex->scale = scale;
        }
    }
}

static void handle_simple_cluster(const HexCoord* hex_coords, size_t num_coords) {
    ASSERT(num_coords >= 3);

    if (game->combos_remaining > 0) {
        game->combos_remaining--;
    }

    for (size_t i = 0; i < num_coords; i++) {
        HexCoord c = hex_coords[i];
        Hex* hex = hex_at(c.q, c.r);
        hex->is_matched = true;
    }

    // Compute score
    //
    // Basic:       (size - 2) * 5 * level
    // Multiplier:  (size - 2) * 100 * level
    // Starflower:  (size - 2) * 2500 * level
    // Black Pearl: (size - 2) * 25000 * level
    int score_multiplier = 0;
    const Hex* hex = hex_at(hex_coords[0].q, hex_coords[0].r);
    if (hex_is_basic(hex)) {
        score_multiplier = 5;
    } else if (hex_is_multiplier(hex)) {
        score_multiplier = 100;
    } else if (hex_is_starflower(hex)) {
        score_multiplier = 2500;
    } else if (hex_is_black_pearl(hex)) {
        // TODO - set flag to end game
        score_multiplier = 25000;
    } else {
        ASSERT(false && "Unknown hex type");
    }
    int local_score = (num_coords - 2) * score_multiplier * game->level;
    game->score += local_score;

    // Start cluster match animation for each hex in cluster
    for (size_t i = 0; i < num_coords; i++) {
        HexCoord c = hex_coords[i];
        ClusterMatchAnimation cma = {
            .in_progress = true,
            .start_time = g_state.frame_count,
        };
        hex_at(c.q, c.r)->cluster_match_animation = cma;
    }

    // Start local score animation
    Rectangle r = hex_bounding_box_of_coords(hex_coords, num_coords);
    Point cluster_center = {
        .x = r.top_left.x + r.width / 2,
        .y = r.top_left.y + r.height / 2,
    };
    LocalScoreAnimation lsa = {
        .in_progress = true,
        .start_time = g_state.frame_count,
        .score = (uint32_t)local_score,
        .alpha = 1.0f,
        .start_point = cluster_center,
        .current_point = cluster_center,
    };
    text_init(&lsa.text);
    vector_push_back(game->local_score_animations, &lsa);
}

// Computes score, updates combos remaining, marks hexes as matched,
// and starts flower animation
static void handle_flower(const HexCoord* hex_coords, size_t num_coords) {
    ASSERT(num_coords == 7);

    if (game->combos_remaining > 0) {
        game->combos_remaining--;
    }

    Hex* center_hex = hex_at(hex_coords[0].q, hex_coords[0].r);
    bool starflower_center = hex_is_starflower(center_hex);
    bool black_pearl_center = hex_is_black_pearl(center_hex);

    center_hex->is_matched = true;
    center_hex->is_flower_matched = true;

    size_t num_neighbor_multipliers = 0;
    bool all_neighbors_starflower = true;
    bool all_neighbors_black_pearl = true;
    for (size_t i = 0; i < 6; i++) {
        Hex* n_hex = hex_at(hex_coords[i+1].q, hex_coords[i+1].r);
        num_neighbor_multipliers += (hex_is_multiplier(n_hex) ? 1 : 0);
        if (!hex_is_starflower(n_hex)) {
            all_neighbors_starflower = false;
        }
        if (!hex_is_black_pearl(n_hex)) {
            all_neighbors_black_pearl = false;
        }
        n_hex->is_matched = true;
        n_hex->is_flower_matched = true;
    }

    if (all_neighbors_starflower) {
        uint32_t mask = (1 << HEX_TYPE_BLACK_PEARL_UP) | (1 << HEX_TYPE_BLACK_PEARL_DOWN);
        center_hex->type = hex_random_type_with_mask(mask);
    } else if (all_neighbors_black_pearl) {
        // TODO - set flag to end game
    } else {
        audio_play_sound_effect(AUDIO_STARFLOWER);
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

    // Start flower match animation for each neighbor
    Point flower_center = (Point){
        .x = center_hex->hex_point.x + HEX_WIDTH / 2,
        .y = center_hex->hex_point.y + HEX_HEIGHT / 2,
    };
    FlowerMatchAnimation fma = {
        .in_progress = true,
        .start_time = g_state.frame_count,
        .flower_center = flower_center,
        .is_center = false,
    };
    for (size_t i = 0; i < 6; i++) {
        Hex* n_hex = hex_at(hex_coords[i+1].q, hex_coords[i+1].r);
        n_hex->flower_match_animation = fma;
    }

    // Start flower match animation for center
    fma.is_center = true;
    center_hex->flower_match_animation = fma;

    // Start local score animation
    LocalScoreAnimation lsa = {
        .in_progress = true,
        .start_time = g_state.frame_count,
        .score = (uint32_t)local_score,
        .alpha = 1.0f,
        .start_point = flower_center,
        .current_point = flower_center,
    };
    text_init(&lsa.text);
    vector_push_back(game->local_score_animations, &lsa);
}

static void handle_gravity(void) {
    uint32_t now = g_state.frame_count;
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = HEX_NUM_ROWS - 1; r >= 0; r--) {
            Hex* hex = hex_at(q, r);
            if (now < hex->gravity_start_time) {
                continue;
            }

            hex->velocity = MIN(MAX_VELOCITY, hex->velocity + game->gravity);
            hex->hex_point.y += hex->velocity;

            const int final_y = transform_hex_to_screen(q, r).y;

            if (hex->hex_point.y >= final_y) {
                // We've reached the final position
                hex->velocity = 0.0f;
                hex->hex_point.y = final_y;
                hex->is_stationary = true;
            } else {
                hex->is_stationary = false;

                // Still falling - check for collisions with the hex (or floor) below.
                int y_below = (r == HEX_NUM_ROWS - 1) ?
                    final_y + HEX_HEIGHT : // floor
                    hex_at(q, r+1)->hex_point.y;

                // Check if bottom of this hex is >= the y coord of the hex (or floor) below
                bool collided = ((hex->hex_point.y + HEX_HEIGHT) >= y_below);
                if (collided) {
                    hex->velocity = 0.0f;
                    hex->hex_point.y = y_below - HEX_HEIGHT;
                }
            }
        }
    }

    if (hex_all_stationary_no_animation()) {
        game->gravity = GRAVITY_NORMAL;
    }
}

// Checking for combos is done in this order:
//  * Flowers
//  * Simple clusters (3, 4, or 5 of the same hex type, or multipliers clusters of any color)
//  * Bomb diffusals (if combined with a multiplier, this will eliminate all of that color)
//  * MMC clusters (whatever clusters remain, containing a mix of basic colors and multiplers)
static void check_for_matches(void) {
    size_t iteration = 0;
    // Match flowers
    Vector flower = vector_create_with_allocator(
            sizeof(HexCoord), bump_allocator_alloc, bump_allocator_free);
    vector_reserve(flower, 7);
    while (1) {
        vector_clear(flower);
        size_t flower_size = hex_find_one_flower(flower);
        if (flower_size == 0) {
            break;
        }
        handle_flower(vector_data_at(flower, 0), vector_size(flower));
        ASSERT(iteration++ < 100);
    }

    // Match simple clusters
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
        // vector_print(simple_cluster, hex_coord_print);
        handle_simple_cluster(vector_data_at(simple_cluster, 0), vector_size(simple_cluster));
        ASSERT(iteration++ < 100);
    }

    // TODO - Match bomb cluster
    // TODO - Match MMCs
}

bool game_update(void) {
    if (g_state.suspend_game) {
        return false;
    }

    // throttle to 1/12 of 60 Hz == 5 Hz
    if (g_state.slow_mode) {
        g_state.slow_mode_throttle++;
        if (g_state.slow_mode_throttle < 12) {
            return false;
        } else {
            g_state.slow_mode_throttle = 0;
        }
    }

    handle_input();
    handle_rotation();
    handle_local_score_animations();
    handle_flower_match_animations();
    handle_cluster_match_animations();
    handle_gravity();
    check_for_matches();

    return true;
}

bool game_init(void) {
    game->seed = now_ms();
    srand(game->seed);

    game->level = 1;
    game->score = 0;
    game->combos_remaining = 50;
    game->gravity = GRAVITY_INITIAL;

    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        game->hexes[q] = vector_create(sizeof(Hex));
        vector_reserve(game->hexes[q], HEX_NUM_ROWS);
    }

    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            hex_spawn(q);
        }
    }

    int reroll_attempts = 0;
    while (board_has_any_matches(false)) {
        reroll_attempts++;
        ASSERT(reroll_attempts < 100);

        // Fix cluster matches by rerolling (q,r)
        for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
            for (int r = 0; r < HEX_NUM_ROWS; r++) {
                if (hex_has_cluster_match(q, r, NULL, NULL, false)) {
                    hex_at(q,r)->type = hex_random_type();
                }
            }
        }

        // Fix starflower matches by rerolling top neighbor
        for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
            for (int r = 0; r < HEX_NUM_ROWS; r++) {
                if (hex_has_flower_match(q, r, false)) {
                    hex_at(q, r-1)->type = hex_random_type();
                }
            }
        }
    }

    // Trigger hexes to fall from above board column by column, left-to-right.
    uint32_t now = g_state.frame_count;
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        uint32_t column_start_time = now + ms_to_frames(500) + q * ms_to_frames(350);
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            Hex* hex = hex_at(q, r);
            hex->gravity_start_time = column_start_time + (HEX_NUM_ROWS - r - 1) * ms_to_frames(100);
        }
    }

    // For testing - load a specific board
    // test_boards_load(g_test_board_six_black_pearls);
    test_boards_load(g_test_board_yellow_starflower);

    cursor_init(&g_state.cursor);

    game->local_score_animations = vector_create(sizeof(LocalScoreAnimation));
    vector_reserve(game->local_score_animations, 10);

    return true;
}
