#include "game_state.h"
#include "math.h"
#include <time.h>
#include <stdlib.h>
#include <assert.h>

#define ROTATION_TIME_MS 150
#define ROTATION_MAX_SCALE 1.5f

// Bitmask to select specific neighbors in the bottom 6 bits.
// Bit index corresponds to HexNeighborID (e.g. bit 5 is top, bit 4 is top right, etc).
#define ALL_NEIGHBORS              0x3F
#define BLACK_PEARL_UP_NEIGHBORS   0x2A
#define BLACK_PEARL_DOWN_NEIGHBORS 0x15
#define TRIO_LEFT_NEIGHBORS        0x03
#define TRIO_RIGHT_NEIGHBORS       0x18

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
    for (int id = HEX_NEIGHBOR_TOP; id >= 0; id--) {
        if (mask & (1 << id)) {
            neighbors->coords[neighbors->num_neighbors++] = neighbor_coord(q, r, id);
        }
    }
}

// Query, to detemine if a hex of specified type at (q,r) would produce a match
static bool hex_would_match(HexType type, int q, int r) {
    HexNeighbors neighbors = {0};
    get_hex_neighbors(q, r, &neighbors, ALL_NEIGHBORS);
    for (int i = 0; i < neighbors.num_neighbors; i++) {
        HexCoord c1 = neighbors.coords[i];
        HexCoord c2 = neighbors.coords[(i + 1) % neighbors.num_neighbors];
        if (!hex_coord_is_valid(c1) || !hex_coord_is_valid(c2)) {
            continue;
        }
        if ((g_state.hexes[c1.q][c1.r].type == type) && (g_state.hexes[c2.q][c2.r].type == type)) {
            return true;
        }
    }
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
    hex->type = rand_in_range(HEX_TYPE_GREEN, NUM_HEX_TYPES-1);
    hex->hex_point = transform_hex_to_screen(q, r);
    hex->scale = 1.0f;
    hex->rotation_angle = 0.0f;
    hex->alpha = 1.0f;
}

static void cursor_update_screen_point(void) {
    Cursor* cursor = &g_state.cursor;

    SDL_Log("Cursor pos %d, (q,r) = (%d,%d)", cursor->position, cursor->hex_anchor.q, cursor->hex_anchor.r);

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

    // Re-roll some hexes to avoid creating matches from the start
    // TODO - Prevent starflowers on inital spawn
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            Hex* hex = &g_state.hexes[q][r];
            int iteration = 0;
            while (hex_would_match(hex->type, q, r)) {
                assert(iteration < 100);
                hex->type = rand_in_range(HEX_TYPE_GREEN, NUM_HEX_TYPES-1);
                iteration++;
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
        if ((type == HEX_TYPE_STAR_FLOWER) ||
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
        if ((type == HEX_TYPE_STAR_FLOWER) ||
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
    const Cursor* cursor = &g_state.cursor;
    const int q = cursor->hex_anchor.q;
    const int r = cursor->hex_anchor.r;

    if (cursor->position == CURSOR_POS_ON) {
        const HexType type = g_state.hexes[q][r].type;
        if (type == HEX_TYPE_STAR_FLOWER) {
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

static void handle_rotation(void) {
    HexNeighbors neighbors = {0};
    get_cursor_neighbors(&neighbors);

    SDL_Log("Neighbors of (%d, %d):", g_state.cursor.hex_anchor.q, g_state.cursor.hex_anchor.r);
    for (int i = 0; i < neighbors.num_neighbors; i++) {
        SDL_Log("   (%d, %d):", neighbors.coords[i].q, neighbors.coords[i].r);
    }

    // TODO - handle more than 3 neighbors
    Hex* hex0 = &g_state.hexes[g_state.cursor.hex_anchor.q][g_state.cursor.hex_anchor.r];
    Hex* hex1 = &g_state.hexes[neighbors.coords[0].q][neighbors.coords[0].r];
    Hex* hex2 = &g_state.hexes[neighbors.coords[1].q][neighbors.coords[1].r];

    double rotation_progress =
        (double)(SDL_GetTicks() - game->rotation_start_time) / (double)ROTATION_TIME_MS;
    if (rotation_progress > 1.0f) {
        g_state.game.rotation_in_progress = false;

        // TODO - replace with rotate_hexes()
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
