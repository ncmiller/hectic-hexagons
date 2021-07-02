#include "game_state.h"
#include "math.h"
#include <time.h>
#include <stdlib.h>
#include <assert.h>

#define ROTATION_TIME_MS 150
#define ROTATION_MAX_SCALE 1.5f

// Convenience accessors to global state
static Game* game = &g_state.game;
static Constants* constants = &g_state.constants;
static Input* input = &g_state.input;

// Includes min and max
static int rand_in_range(int min, int max) {
    return min + (rand() % (max - min + 1));
}

static void initialize_constants(void) {
    const double s = HEX_WIDTH / 2.0f;
    const double h = sqrt(3) * s;
    const double w = 2.0f * s;
    constants->hex_s = s;
    constants->hex_h = h;
    constants->hex_w = w;

    constants->board_width = 0.75f * w * HEX_NUM_COLUMNS;
    constants->board_height = h * HEX_NUM_ROWS;
    constants->board.x = LOGICAL_WINDOW_WIDTH / 2 - constants->board_width / 2;
    constants->board.y = LOGICAL_WINDOW_HEIGHT / 2 - constants->board_height / 2;
}

// Compute screen space point of upper-left corner of hex
static Point transform_hex_to_screen(int q, int r) {
    Point p;
    bool q_even = ((q & 1) == 0);
    p.x = constants->board.x + (int)round(q * 0.75f * constants->hex_w);
    if (q_even) {
        p.y = constants->board.y + (int)round(constants->hex_h * (r + 0.5f));
    } else {
        p.y = constants->board.y + (int)round(constants->hex_h * r);
    }
    return p;
}

static Point transform_cursor_to_screen(int q, int r) {
    assert(q >= 0 && q < CURSOR_NUM_COLUMNS);
    assert(r >= 0 && r < CURSOR_NUM_ROWS);

    Point p;
    p.y = constants->hex_h * (1 + r * 0.5f);

    // Calculating x position is a little tricky, so use lookup tables
    const double even_row_x_scalars[CURSOR_NUM_COLUMNS] = {
        1.00f, 1.50f, 2.50f, 3.00f, 4.00f, 4.50f, 5.50f, 6.00f, 7.00f
    };
    const double odd_row_x_scalars[CURSOR_NUM_COLUMNS] = {
        0.75f, 1.75f, 2.25f, 3.25f, 3.75f, 4.75f, 5.25f, 6.25f, 6.75f
    };

    if (r & 1) {
        p.x = odd_row_x_scalars[q] * constants->hex_w;
    } else {
        p.x = even_row_x_scalars[q] * constants->hex_w;
    }

    p.x += constants->board.x;
    p.y += constants->board.y;

    return p;
}

// Returns valid hex neighbors of hex (q, r) in clockwise order
static void get_hex_neighbors(int q, int r, HexNeighbors* neighbors) {
    // Top
    if (q > 0) {
        neighbors->coords[neighbors->num_neighbors].q = q - 1;
        neighbors->coords[neighbors->num_neighbors].r = r;
        neighbors->num_neighbors++;
    }

    // Top-right

    // Bottom-right
    // Bottom
    // Bottom-left
    // Top-left
}

// Query, to detemine if a hex of specified type at (q,r) would produce a match
static bool hex_would_match(HexType type, int q, int r) {
    HexNeighbors neighbors;
    get_hex_neighbors(q, r, &neighbors);
    return false;
}

static void spawn_hex(int q, int r, bool allow_match) {
    assert(q < HEX_NUM_COLUMNS && r < HEX_NUM_ROWS);
    Hex* hex = &g_state.hexes[q][r];

    // For even columns, the last row of hexes are not valid
    bool q_even = ((q & 1) == 0);
    if (q_even && (r == HEX_NUM_ROWS - 1)) {
        hex->is_valid = false;
        return;
    }

    hex->is_valid = true;
    HexType random_type = rand_in_range(HEX_TYPE_GREEN, HEX_TYPE_RED);
    if (!allow_match) {
        // Resample until we find a type that doesn't match neighbors
        int iteration = 0;
        while (hex_would_match(random_type, q, r)) {
            assert(iteration < 100);
            random_type = rand_in_range(HEX_TYPE_GREEN, HEX_TYPE_RED);
            iteration++;
        }
    }
    hex->type = random_type;
    hex->hex_point = transform_hex_to_screen(q, r);
    hex->scale = 1.0f;
    hex->rotation_angle = 0.0f;
    hex->alpha = 1.0f;
}

void set_cursor_position(int q, int r) {
    if (q < 0 || r < 0 || q >= CURSOR_NUM_COLUMNS || r >= CURSOR_NUM_ROWS) {
        return;
    }
    g_state.cursor.coord.q = q;
    g_state.cursor.coord.r = r;
    g_state.cursor.screen_point = transform_cursor_to_screen(q, r);
}

bool game_init(void) {
    initialize_constants();

    srand(time(0));

    g_state.game.level = 1;
    g_state.game.combos_remaining = 50;
    g_state.game.score = 0;
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            spawn_hex(q, r, false);
        }
    }

    set_cursor_position(CURSOR_NUM_COLUMNS / 2, CURSOR_NUM_ROWS / 2);
    g_state.cursor.is_visible = true;

    return true;
}

static void handle_input(void) {
    if (game->rotation_in_progress) {
        return;
    }

    if (input->up) {
        input->up = false;
        set_cursor_position(g_state.cursor.coord.q, g_state.cursor.coord.r - 1);
    } else if (input->down) {
        input->down = false;
        set_cursor_position(g_state.cursor.coord.q, g_state.cursor.coord.r + 1);
    } else if (input->right) {
        input->right = false;
        set_cursor_position(g_state.cursor.coord.q + 1, g_state.cursor.coord.r);
    } else if (input->left) {
        input->left = false;
        set_cursor_position(g_state.cursor.coord.q - 1, g_state.cursor.coord.r);
    }

    bool start_rotation = input->rotate_cw || input->rotate_ccw;
    if (start_rotation && !game->rotation_in_progress) {
        game->rotation_in_progress = true;
        if (input->rotate_cw) {
            input->rotate_cw = false;
            game->degrees_to_rotate = 120.0f;
        } else if (g_state.input.rotate_ccw) {
            input->rotate_ccw = false;
            game->degrees_to_rotate = -120.0f;
        }
        game->rotation_start_time = SDL_GetTicks();
    }
}

static void get_cursor_neighbors(HexNeighbors* neighbors) {
    const CursorCoord* cursor = &g_state.cursor.coord;
    bool r_odd = (cursor->r & 1);
    bool q_odd = (cursor->q & 1);

    // There's probably a smarter way to do this but this works...
    if (!q_odd && !r_odd) {
        HexCoord left_hex = {0};
        left_hex.q = cursor->q;
        left_hex.r = cursor->r / 2;

        neighbors->coords[neighbors->num_neighbors].q = left_hex.q;
        neighbors->coords[neighbors->num_neighbors].r = left_hex.r;
        neighbors->num_neighbors++;

        neighbors->coords[neighbors->num_neighbors].q = left_hex.q + 1;
        neighbors->coords[neighbors->num_neighbors].r = left_hex.r;
        neighbors->num_neighbors++;

        neighbors->coords[neighbors->num_neighbors].q = left_hex.q + 1;
        neighbors->coords[neighbors->num_neighbors].r = left_hex.r + 1;
        neighbors->num_neighbors++;
    } else if (!q_odd && r_odd) {
        HexCoord right_hex = {0};
        right_hex.q = cursor->q + 1;
        right_hex.r = cursor->r / 2 + 1;

        neighbors->coords[neighbors->num_neighbors].q = right_hex.q;
        neighbors->coords[neighbors->num_neighbors].r = right_hex.r;
        neighbors->num_neighbors++;

        neighbors->coords[neighbors->num_neighbors].q = right_hex.q - 1;
        neighbors->coords[neighbors->num_neighbors].r = right_hex.r;
        neighbors->num_neighbors++;

        neighbors->coords[neighbors->num_neighbors].q = right_hex.q - 1;
        neighbors->coords[neighbors->num_neighbors].r = right_hex.r - 1;
        neighbors->num_neighbors++;
    } else if (q_odd && !r_odd) {
        HexCoord right_hex = {0};
        right_hex.q = cursor->q + 1;
        right_hex.r = cursor->r / 2;

        neighbors->coords[neighbors->num_neighbors].q = right_hex.q;
        neighbors->coords[neighbors->num_neighbors].r = right_hex.r;
        neighbors->num_neighbors++;

        neighbors->coords[neighbors->num_neighbors].q = right_hex.q - 1;
        neighbors->coords[neighbors->num_neighbors].r = right_hex.r + 1;
        neighbors->num_neighbors++;

        neighbors->coords[neighbors->num_neighbors].q = right_hex.q - 1;
        neighbors->coords[neighbors->num_neighbors].r = right_hex.r;
        neighbors->num_neighbors++;
    } else { // q_odd && r_odd
        HexCoord left_hex = {0};
        left_hex.q = cursor->q;
        left_hex.r = cursor->r / 2 + 1;

        neighbors->coords[neighbors->num_neighbors].q = left_hex.q;
        neighbors->coords[neighbors->num_neighbors].r = left_hex.r;
        neighbors->num_neighbors++;

        neighbors->coords[neighbors->num_neighbors].q = left_hex.q + 1;
        neighbors->coords[neighbors->num_neighbors].r = left_hex.r - 1;
        neighbors->num_neighbors++;

        neighbors->coords[neighbors->num_neighbors].q = left_hex.q + 1;
        neighbors->coords[neighbors->num_neighbors].r = left_hex.r;
        neighbors->num_neighbors++;
    }

    // TODO - handle starflower and black pearl
}

static void handle_rotation(void) {
    HexNeighbors neighbors = {0};
    get_cursor_neighbors(&neighbors);

    Hex* hex0 = &g_state.hexes[neighbors.coords[0].q][neighbors.coords[0].r];
    Hex* hex1 = &g_state.hexes[neighbors.coords[1].q][neighbors.coords[1].r];
    Hex* hex2 = &g_state.hexes[neighbors.coords[2].q][neighbors.coords[2].r];

    double rotation_progress =
        (double)(SDL_GetTicks() - game->rotation_start_time) / (double)ROTATION_TIME_MS;
    if (rotation_progress > 1.0f) {
        g_state.game.rotation_in_progress = false;

        hex0->is_rotating = false;
        hex1->is_rotating = false;
        hex2->is_rotating = false;

        hex0->rotation_angle = 0.0f;
        hex1->rotation_angle = 0.0f;
        hex2->rotation_angle = 0.0f;

        hex0->scale = 1.0f;
        hex1->scale = 1.0f;
        hex2->scale = 1.0f;

        HexType temp1 = hex1->type;
        HexType temp2 = hex2->type;
        if (g_state.game.degrees_to_rotate > 0) {
            hex1->type = hex0->type;
            hex2->type = temp1;
            hex0->type = temp2;
        } else {
            hex1->type = temp2;
            hex2->type = hex0->type;
            hex0->type = temp1;
        }
    } else {
        hex0->is_rotating = true;
        hex1->is_rotating = true;
        hex2->is_rotating = true;

        double angle = rotation_progress * g_state.game.degrees_to_rotate;
        hex0->rotation_angle = angle;
        hex1->rotation_angle = angle;
        hex2->rotation_angle = angle;

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

        hex0->scale = scale;
        hex1->scale = scale;
        hex2->scale = scale;
    }
}

void game_update(void) {
    handle_input();

    if (g_state.game.rotation_in_progress) {
        handle_rotation();
    }
}
