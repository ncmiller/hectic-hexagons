#include "hex.h"
#include "game_state.h"
#include "constants.h"
#include <assert.h>
#include <math.h>

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

// Compute screen space point of upper-left corner of hex
static Point transform_hex_to_screen(int q, int r) {
    Point p;
    bool q_even = ((q & 1) == 0);
    p.x = g_constants.board.x + (int)round(q * 0.75f * g_constants.hex_w);
    if (q_even) {
        p.y = g_constants.board.y + (int)round(g_constants.hex_h * (r + 0.5f));
    } else {
        p.y = g_constants.board.y + (int)round(g_constants.hex_h * r);
    }
    return p;
}

HexCoord hex_neighbor_coord(int q, int r, HexNeighborID neighbor_id) {
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

void hex_neighbors(int q, int r, HexNeighbors* neighbors, uint8_t mask) {
    for (int id = 0; id < MAX_NUM_HEX_NEIGHBORS; id++) {
        if (mask & (1 << id)) {
            neighbors->coords[neighbors->num_neighbors++] = hex_neighbor_coord(q, r, id);
        }
    }
}

void hex_spawn(int q, int r) {
    assert(q < HEX_NUM_COLUMNS && r < HEX_NUM_ROWS);
    Hex* hex = hex_at(q, r);

    // For even columns, the last row of hexes are not valid
    bool q_even = ((q & 1) == 0);
    if (q_even && (r == HEX_NUM_ROWS - 1)) {
        hex->is_valid = false;
        return;
    }

    hex->is_valid = true;
    hex->type = hex_random_type();
    hex->hex_point = transform_hex_to_screen(q, r);
    hex->scale = 1.0f;
    hex->rotation_angle = 0.0f;
    hex->alpha = 1.0f;
}

Hex* hex_at(int q, int r) {
    return &g_state.hexes[q][r];
}

HexType hex_random_type(void) {
    return hex_random_type_with_mask(LEVEL_HEX_TYPE_MASK[g_state.game.level]);
}

HexType hex_random_type_with_mask(uint32_t mask) {
    // TODO - support for different odds of rolling multipliers and bombs
    //
    // For Hexic HD, people online report roughly 3% drop rate for multipliers.
    // Not sure about bombs.
    HexType allowed_types[NUM_HEX_TYPES] = {0};
    size_t num_allowed_types = 0;
    for (int bit = 0; bit < NUM_HEX_TYPES; bit++) {
        if ((1 << bit) & mask) {
            allowed_types[num_allowed_types++] = bit;
        }
    }
    int rand_allowed_type_index = rand() % num_allowed_types;
    return allowed_types[rand_allowed_type_index];
}

bool hex_has_cluster_match(int q, int r) {
    const Hex* query_hex = &g_state.hexes[q][r];
    HexNeighbors neighbors = {0};
    hex_neighbors(q, r, &neighbors, ALL_NEIGHBORS);
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


bool hex_has_flower_match(int q, int r) {
    HexNeighbors neighbors = {0};
    hex_neighbors(q, r, &neighbors, ALL_NEIGHBORS);

    // Check that all neighbors are valid
    for (int i = 0; i < neighbors.num_neighbors; i++) {
        HexCoord c = neighbors.coords[i];
        if (!hex_coord_is_valid(c)) {
            return false;
        }
    }

    // Check that all neighbors have either:
    //   1. all the same type
    //   2. all the same color (mix of basic, multiplier, bomb)
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

size_t hex_find_one_flower(Vector* hex_coords) {
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            if (hex_at(q, r)->is_matched) {
                continue;
            }

            if (hex_has_flower_match(q, r)) {
                HexCoord c = (HexCoord){ .q = q, .r = r };
                vector_push_back(*hex_coords, &c);

                HexNeighbors neighbors = {0};
                hex_neighbors(q, r, &neighbors, ALL_NEIGHBORS);
                assert(neighbors.num_neighbors == 6);
                for (int i = 0; i < 6; i++) {
                    vector_push_back(*hex_coords, &neighbors.coords[i]);
                }
                return 7;
            }
        }
    }
    return 0;
}

size_t hex_find_one_simple_cluster(Vector* hex_coords) {
    // TODO
    return 0;
}

size_t hex_find_one_bomb_cluster(Vector* hex_coords) {
    // TODO
    return 0;
}

size_t hex_find_one_mmc_cluster(Vector* hex_coords) {
    // TODO
    return 0;
}

bool hex_is_black_pearl(const Hex* hex) {
    return (hex->type == HEX_TYPE_BLACK_PEARL_UP) || (hex->type == HEX_TYPE_BLACK_PEARL_DOWN);
}

bool hex_is_bomb(const Hex* hex) {
    // TODO
    return false;
}

bool hex_is_basic(const Hex* hex) {
    return (hex->type >= HEX_TYPE_GREEN && hex->type <= HEX_TYPE_RED);
}

bool hex_is_multiplier(const Hex* hex) {
    // TODO
    return false;
}

bool hex_is_starflower(const Hex* hex) {
    return (hex->type == HEX_TYPE_STARFLOWER);
}

void hex_clear_is_matched(Hex* hex) {
    hex->is_matched = false;
}

void hex_for_each(HexFn fn) {
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            fn(hex_at(q, r));
        }
    }
}
