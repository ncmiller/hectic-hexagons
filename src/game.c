#include "game_state.h"
#include "math.h"
#include <time.h>
#include <stdlib.h>
#include <assert.h>

#define ROTATION_TIME_MS 150
#define ROTATION_MAX_SCALE 1.5f

// Bitmask to select specific neighbors in the bottom 6 bits.
// Bit index corresponds to HexNeighborID (e.g. bit 0 is top, bit 1 is top right, etc).
#define ALL_NEIGHBORS              0x3F
#define BLACK_PEARL_UP_NEIGHBORS   0x15
#define BLACK_PEARL_DOWN_NEIGHBORS 0x2A
#define TRIO_LEFT_NEIGHBORS        0x30
#define TRIO_RIGHT_NEIGHBORS       0x06

// Bitmasks to select specific hex types to spawn, based on level.
// Bit index corresponds to HexType (e.g. bit 0 is green, bit 1 is blue, etc).
#define MAX_NUM_LEVELS 7
static const uint32_t LEVEL_HEX_TYPE_MASK[MAX_NUM_LEVELS + 1] = {
    [0] = 0x00, // invalid, not a level
    [1] = 0x37, // level 1, no magenta
    [2] = 0x37, // level 2, add multipliers
    [3] = 0x37, // level 3, add bombs
    [4] = 0x3F, // level 4, add magenta
    [5] = 0x3F, // level 5, no change
    [6] = 0x3F, // level 6, no change
    [7] = 0x3F, // level 7, no change
};

// Convenience accessors to global state
static Game* game = &g_state.game;
static Constants* constants = &g_state.constants;
static Input* input = &g_state.input;

static HexType random_hex_type() {
    // TODO - support for different odds of rolling multipliers and bombs
    HexType allowed_types[NUM_HEX_TYPES] = {0};
    size_t num_allowed_types = 0;
    for (int bit = 0; bit < NUM_HEX_TYPES; bit++) {
        uint32_t mask = LEVEL_HEX_TYPE_MASK[g_state.game.level];
        if ((1 << bit) & mask) {
            allowed_types[num_allowed_types++] = bit;
        }
    }
    int rand_allowed_type_index = rand() % num_allowed_types;
    return allowed_types[rand_allowed_type_index];
}

static void initialize_constants(void) {
    const double s = HEX_WIDTH / 2.0f;
    const double h = sqrt(3) * s;
    const double w = 2.0f * s;
    constants->hex_s = s;
    constants->hex_h = h;
    constants->hex_w = w;

    constants->board_width = 5.0f * w + 4.0 * (w / 2.0f) + 0.75f * w;
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

static bool hex_coord_is_valid(HexCoord coord) {
    if (coord.q < 0 || coord.r < 0) {
        return false;
    }
    if (coord.q >= HEX_NUM_COLUMNS || coord.r >= HEX_NUM_ROWS) {
        return false;
    }
    if ((coord.r == HEX_NUM_ROWS - 1) && ((coord.q & 1) == 0)) {
        return false;
    }
    return true;
}

static HexCoord neighbor_coord(int q, int r, HexNeighborID neighbor_id) {
    bool q_odd = (q & 1);
    HexCoord coord = {0};

    switch (neighbor_id) {
        case HEX_NEIGHBOR_TOP:
            coord.q = q;
            coord.r = r - 1;
            break;
        case HEX_NEIGHBOR_TOP_RIGHT:
            coord.q = q + 1;
            coord.r = (q_odd ? r - 1 : r);
            break;
        case HEX_NEIGHBOR_BOTTOM_RIGHT:
            coord.q = q + 1;
            coord.r = (q_odd ? r : r + 1);
            break;
        case HEX_NEIGHBOR_BOTTOM:
            coord.q = q;
            coord.r = r + 1;
            break;
        case HEX_NEIGHBOR_BOTTOM_LEFT:
            coord.q = q - 1;
            coord.r = (q_odd ? r : r + 1);
            break;
        case HEX_NEIGHBOR_TOP_LEFT:
            coord.q = q - 1;
            coord.r = (q_odd ? r - 1 : r);
            break;
        default:
            SDL_Log("Invalid neighbor ID %d", neighbor_id);
            assert(false);
            break;
    }
    return coord;
}

// Returns hex neighbors of hex (q, r) in clockwise order.
// Neighbor is only added if the corresponding HexNeighborID bit is set in the mask.
// Note that some of these may be invalid or outside of bounds.
// Caller should check them with hex_coord_is_valid(q, r).
static void get_hex_neighbors(int q, int r, HexNeighbors* neighbors, uint8_t mask) {
    for (int id = 0; id < MAX_NUM_HEX_NEIGHBORS; id++) {
        if (mask & (1 << id)) {
            neighbors->coords[neighbors->num_neighbors++] = neighbor_coord(q, r, id);
        }
    }
}

// Query, to detemine if the hex at (q,r) would produce a trio match
static bool hex_trio_match(int q, int r) {
    const Hex* query_hex = &g_state.hexes[q][r];
    HexNeighbors neighbors = {0};
    get_hex_neighbors(q, r, &neighbors, ALL_NEIGHBORS);
    for (int i = 0; i < neighbors.num_neighbors; i++) {
        HexCoord c1 = neighbors.coords[i];
        HexCoord c2 = neighbors.coords[(i + 1) % neighbors.num_neighbors];
        if (!hex_coord_is_valid(c1) || !hex_coord_is_valid(c2)) {
            continue;
        }
        const Hex* hex1 = &g_state.hexes[c1.q][c1.r];
        const Hex* hex2 = &g_state.hexes[c2.q][c2.r];
        if ((hex1->type == query_hex->type) && (hex2->type == query_hex->type)) {
            return true;
        }
    }
    return false;
}

// Query, to determine if the hex at (q,r) would produce a starflower match (all neighbors same)
static bool hex_starflower_match(int q, int r) {
    HexNeighbors neighbors = {0};
    get_hex_neighbors(q, r, &neighbors, ALL_NEIGHBORS);

    // Check that all neighbors are valid
    for (int i = 0; i < neighbors.num_neighbors; i++) {
        HexCoord c = neighbors.coords[i];
        if (!hex_coord_is_valid(c)) {
            return false;
        }
    }

    // Check that all neighbors have the same type
    HexType type = g_state.hexes[neighbors.coords[0].q][neighbors.coords[0].r].type;
    for (int i = 1; i < neighbors.num_neighbors; i++) {
        HexCoord neighbor_coord = neighbors.coords[i];
        HexType neighbor_type = g_state.hexes[neighbor_coord.q][neighbor_coord.r].type;
        if (neighbor_type != type) {
            return false;
        }
    }
    return true;
}

static void spawn_hex(int q, int r) {
    assert(q < HEX_NUM_COLUMNS && r < HEX_NUM_ROWS);
    Hex* hex = &g_state.hexes[q][r];

    // For even columns, the last row of hexes are not valid
    bool q_even = ((q & 1) == 0);
    if (q_even && (r == HEX_NUM_ROWS - 1)) {
        hex->is_valid = false;
        return;
    }

    hex->is_valid = true;
    hex->type = random_hex_type();
    hex->hex_point = transform_hex_to_screen(q, r);
    hex->scale = 1.0f;
    hex->rotation_angle = 0.0f;
    hex->alpha = 1.0f;
}

static void cursor_update_screen_point(void) {
    Cursor* cursor = &g_state.cursor;

    const Hex* hex = &g_state.hexes[cursor->hex_anchor.q][cursor->hex_anchor.r];
    if (cursor->position == CURSOR_POS_RIGHT) {
        cursor->screen_point.x = hex->hex_point.x + HEX_WIDTH;
        cursor->screen_point.y = hex->hex_point.y + HEX_HEIGHT / 2;
    } else if (cursor->position == CURSOR_POS_LEFT) {
        cursor->screen_point.x = hex->hex_point.x;
        cursor->screen_point.y = hex->hex_point.y + HEX_HEIGHT / 2;
    } else if (cursor->position == CURSOR_POS_ON) {
        cursor->screen_point.x = hex->hex_point.x + HEX_WIDTH / 2;
        cursor->screen_point.y = hex->hex_point.y + HEX_HEIGHT / 2;
    }
}

static void spawn_all_hexes(void) {
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            spawn_hex(q, r);
        }
    }
}

static bool board_has_any_trio_matches(void) {
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            if (hex_trio_match(q, r)) {
                return true;
            }
        }
    }
    return false;
}

static bool board_has_any_starflower_matches(void) {
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            if (hex_starflower_match(q, r)) {
                return true;
            }
        }
    }
    return false;
}

static bool board_has_any_matches(void) {
    return board_has_any_trio_matches() || board_has_any_starflower_matches();
}

bool game_init(void) {
    initialize_constants();

    srand(time(0));

    g_state.game.level = 1;
    g_state.game.combos_remaining = 50;
    g_state.game.score = 0;

    spawn_all_hexes();

    int reroll_attempts = 0;
    while (board_has_any_matches()) {
        reroll_attempts++;
        assert(reroll_attempts < 100);

        // Fix trio matches by rerolling (q,r)
        for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
            for (int r = 0; r < HEX_NUM_ROWS; r++) {
                if (hex_trio_match(q, r)) {
                    g_state.hexes[q][r].type = random_hex_type();
                }
            }
        }

        // Fix starflower matches by rerolling top neighbor
        for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
            for (int r = 0; r < HEX_NUM_ROWS; r++) {
                if (hex_starflower_match(q, r)) {
                    g_state.hexes[q][r-1].type = random_hex_type();
                }
            }
        }
    }

    HexCoord cursor_start = { .q = HEX_NUM_COLUMNS / 2, .r = HEX_NUM_ROWS / 2 };
    g_state.cursor.hex_anchor = cursor_start;
    g_state.cursor.position = CURSOR_POS_LEFT;
    cursor_update_screen_point();
    g_state.cursor.is_visible = true;

    return true;
}

static void cursor_up(void) {
    Cursor* cursor = &g_state.cursor;
    int q = cursor->hex_anchor.q;
    int r = cursor->hex_anchor.r;

    if (cursor->position == CURSOR_POS_RIGHT) {
        if (r == 0) {
            return;
        }
        cursor->hex_anchor = neighbor_coord(q, r, HEX_NEIGHBOR_TOP_RIGHT);
        cursor->position = CURSOR_POS_LEFT;
    } else if (cursor->position == CURSOR_POS_LEFT) {
        if (r == 0) {
            return;
        }
        cursor->hex_anchor = neighbor_coord(q, r, HEX_NEIGHBOR_TOP_LEFT);
        cursor->position = CURSOR_POS_RIGHT;
    } else if (cursor->position == CURSOR_POS_ON) {
        cursor->hex_anchor = neighbor_coord(q, r, HEX_NEIGHBOR_TOP_LEFT);
        cursor->position = CURSOR_POS_RIGHT;
    }

    cursor_update_screen_point();
}

static void cursor_down(void) {
    Cursor* cursor = &g_state.cursor;
    int q = cursor->hex_anchor.q;
    int r = cursor->hex_anchor.r;

    if (cursor->position == CURSOR_POS_RIGHT) {
        if (r == HEX_NUM_ROWS - 2) {
            return;
        }
        cursor->hex_anchor = neighbor_coord(q, r, HEX_NEIGHBOR_BOTTOM_RIGHT);
        cursor->position = CURSOR_POS_LEFT;
    } else if (cursor->position == CURSOR_POS_LEFT) {
        if (r == HEX_NUM_ROWS - 1) {
            return;
        }
        cursor->hex_anchor = neighbor_coord(q, r, HEX_NEIGHBOR_BOTTOM_LEFT);
        cursor->position = CURSOR_POS_RIGHT;
    } else if (cursor->position == CURSOR_POS_ON) {
        cursor->hex_anchor = neighbor_coord(q, r, HEX_NEIGHBOR_BOTTOM_LEFT);
        cursor->position = CURSOR_POS_RIGHT;
    }

    cursor_update_screen_point();
}

static void cursor_right(void) {
    Cursor* cursor = &g_state.cursor;
    int q = cursor->hex_anchor.q;

    if (cursor->position == CURSOR_POS_RIGHT) {
        if (q >= HEX_NUM_COLUMNS - 2) {
            return;
        }
        cursor->hex_anchor.q += 2;
        cursor->position = CURSOR_POS_LEFT;
    } else if (cursor->position == CURSOR_POS_LEFT) {
        if (q >= HEX_NUM_COLUMNS - 1) {
            return;
        }
        HexType type = g_state.hexes[cursor->hex_anchor.q][cursor->hex_anchor.r].type;
        if ((type == HEX_TYPE_STARFLOWER) ||
            (type == HEX_TYPE_BLACK_PEARL_UP) ||
            (type == HEX_TYPE_BLACK_PEARL_DOWN)) {
            cursor->position = CURSOR_POS_ON;
        } else {
            cursor->position = CURSOR_POS_RIGHT;
        }
    } else if (cursor->position == CURSOR_POS_ON) {
        cursor->position = CURSOR_POS_RIGHT;
    }

    cursor_update_screen_point();
}

static void cursor_left(void) {
    Cursor* cursor = &g_state.cursor;
    int q = cursor->hex_anchor.q;

    if (cursor->position == CURSOR_POS_RIGHT) {
        if (q == 0) {
            return;
        }
        HexType type = g_state.hexes[cursor->hex_anchor.q][cursor->hex_anchor.r].type;
        if ((type == HEX_TYPE_STARFLOWER) ||
            (type == HEX_TYPE_BLACK_PEARL_UP) ||
            (type == HEX_TYPE_BLACK_PEARL_DOWN)) {
            cursor->position = CURSOR_POS_ON;
        } else {
            cursor->position = CURSOR_POS_LEFT;
        }
    } else if (cursor->position == CURSOR_POS_LEFT) {
        if (q <= 1) {
            return;
        }
        cursor->hex_anchor.q -= 2;
        cursor->position = CURSOR_POS_RIGHT;
    } else if (cursor->position == CURSOR_POS_ON) {
        cursor->position = CURSOR_POS_LEFT;
    }

    cursor_update_screen_point();
}

static void handle_input(void) {
    // Wait for rotation to finish before processing any more inputs
    if (game->rotation_in_progress) {
        return;
    }

    if (input->up) {
        input->up = false;
        cursor_up();
    } else if (input->down) {
        input->down = false;
        cursor_down();
    } else if (input->right) {
        input->right = false;
        cursor_right();
    } else if (input->left) {
        input->left = false;
        cursor_left();
    }

    bool start_rotation = input->rotate_cw || input->rotate_ccw;
    if (start_rotation && !game->rotation_in_progress) {
        game->rotation_in_progress = true;
        game->rotation_start_time = SDL_GetTicks();

        // Determine if this is a starflower rotation
        const Cursor* cursor = &g_state.cursor;
        HexType cursor_hex_type = g_state.hexes[cursor->hex_anchor.q][cursor->hex_anchor.r].type;
        bool is_starflower_rotation =
            (cursor->position == CURSOR_POS_ON) && (cursor_hex_type == HEX_TYPE_STARFLOWER);

        if (input->rotate_cw) {
            input->rotate_cw = false;
            game->degrees_to_rotate = (is_starflower_rotation ? 60.0f : 120.0f);
        } else if (g_state.input.rotate_ccw) {
            input->rotate_ccw = false;
            game->degrees_to_rotate = (is_starflower_rotation ? -60.0f : -120.0f);
        }
    }
}

static void get_cursor_neighbors(HexNeighbors* neighbors) {
    const Cursor* cursor = &g_state.cursor;
    const int q = cursor->hex_anchor.q;
    const int r = cursor->hex_anchor.r;

    if (cursor->position == CURSOR_POS_ON) {
        const HexType type = g_state.hexes[q][r].type;
        if (type == HEX_TYPE_STARFLOWER) {
            get_hex_neighbors(q, r, neighbors, ALL_NEIGHBORS);
        } else if (type == HEX_TYPE_BLACK_PEARL_UP) {
            get_hex_neighbors(q, r, neighbors, BLACK_PEARL_UP_NEIGHBORS);
        } else if (type == HEX_TYPE_BLACK_PEARL_DOWN) {
            get_hex_neighbors(q, r, neighbors, BLACK_PEARL_DOWN_NEIGHBORS);
        }
    } else if (cursor->position == CURSOR_POS_LEFT) {
        get_hex_neighbors(q, r, neighbors, TRIO_LEFT_NEIGHBORS);
    } else if (cursor->position == CURSOR_POS_RIGHT) {
        get_hex_neighbors(q, r, neighbors, TRIO_RIGHT_NEIGHBORS);
    } else {
        assert(false && "Unexpected cursor position");
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
    HexNeighbors neighbors = {0};
    get_cursor_neighbors(&neighbors);

    Cursor* cursor = &g_state.cursor;
    Hex* cursor_hex = &g_state.hexes[cursor->hex_anchor.q][cursor->hex_anchor.r];

    const double rotation_progress =
        (double)(SDL_GetTicks() - game->rotation_start_time) / (double)ROTATION_TIME_MS;

    if (rotation_progress > 1.0f) {
        g_state.game.rotation_in_progress = false;

        cursor_hex->is_rotating = false;
        cursor_hex->rotation_angle = 0.0f;
        cursor_hex->scale = 1.0f;

        for (int i = 0; i < neighbors.num_neighbors; i++) {
            Hex* hex = &g_state.hexes[neighbors.coords[i].q][neighbors.coords[i].r];
            hex->is_rotating = false;
            hex->rotation_angle = 0.0f;
            hex->scale = 1.0f;
        }

        bool is_rotate_clockwise = (g_state.game.degrees_to_rotate > 0);
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
        const double angle = rotation_progress * g_state.game.degrees_to_rotate;
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

void game_update(void) {
    handle_input();

    if (g_state.game.rotation_in_progress) {
        handle_rotation();
    }
}
