#include "hex.h"
#include "game_state.h"
#include "constants.h"
#include "bump_allocator.h"
#include "macros.h"
#include "window.h"
#include <assert.h>
#include <math.h>

bool hex_coord_is_valid(HexCoord coord) {
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

    Hex new_hex = {
        .is_valid = true,
        .type = hex_random_type(),
        .hex_point = transform_hex_to_screen(q, r),
        .scale = 1.0f,
        .rotation_angle = 0.0f,
        .alpha = 1.0f,
    };
    *hex = new_hex;
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

bool hex_has_cluster_match(int q, int r, HexCoord* n1, HexCoord* n2) {
    if (!hex_coord_is_valid((HexCoord){q, r})) {
        return false;
    }

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
            if (n1) {
                *n1 = c1;
            }
            if (n2) {
                *n2 = c2;
            }
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

size_t hex_find_one_flower(Vector hex_coords) {
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            if (hex_at(q, r)->is_matched) {
                continue;
            }

            if (hex_has_flower_match(q, r)) {
                HexCoord c = (HexCoord){ .q = q, .r = r };
                vector_push_back(hex_coords, &c);

                HexNeighbors neighbors = {0};
                hex_neighbors(q, r, &neighbors, ALL_NEIGHBORS);
                assert(neighbors.num_neighbors == 6);
                for (int i = 0; i < 6; i++) {
                    vector_push_back(hex_coords, &neighbors.coords[i]);
                }
                return 7;
            }
        }
    }
    return 0;
}

size_t hex_find_one_simple_cluster(Vector hex_coords) {
    bool in_cluster[HEX_NUM_COLUMNS][HEX_NUM_ROWS] = {0};
    Vector dfs_stack = vector_create_with_allocator(
            sizeof(HexCoord),
            bump_allocator_alloc,
            bump_allocator_free);

    // Iterate over the entire board.
    // Find the first trio cluster (all same type, none previously matched).
    // DFS, starting with the trio, visit neighboring hexes that should be part of the cluster.
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            if (hex_at(q, r)->is_matched) {
                continue;
            }

            HexCoord n1, n2;
            if (!hex_has_cluster_match(q, r, &n1, &n2)) {
                continue;
            }

            if (hex_at(n1.q, n1.r)->is_matched || hex_at(n2.q, n2.r)->is_matched) {
                continue;
            }

            vector_clear(dfs_stack);
            vector_clear(hex_coords);

            HexCoord start = {q,r};
            in_cluster[q][r] = true;
            vector_push_back(dfs_stack, &start);

            in_cluster[n1.q][n1.r] = true;
            vector_push_back(dfs_stack, &n1);

            in_cluster[n2.q][n2.r] = true;
            vector_push_back(dfs_stack, &n2);

            HexType target_type = hex_at(q, r)->type;

            while (vector_size(dfs_stack) > 0) {
                HexCoord c = {0};
                vector_pop_back(dfs_stack, &c);
                // SDL_Log("Pop (%d,%d)", c.q, c.r);
                vector_push_back(hex_coords, &c);

                // Add neighbors to cluster if they meet all criteria:
                //   1. Valid
                //   2. Not matched
                //   3. Not already in cluster
                //   4. Type matches
                //   5. Prior or next neighbor type matches and in cluster
                HexNeighbors neighbors = {0};
                hex_neighbors(c.q, c.r, &neighbors, ALL_NEIGHBORS);
                for (int i = 0; i < neighbors.num_neighbors; i++) {
                    HexCoord c1 = (i == 0 ?
                            neighbors.coords[neighbors.num_neighbors - 1] :
                            neighbors.coords[i - 1]);
                    HexCoord c2 = neighbors.coords[i];
                    HexCoord c3 = neighbors.coords[(i + 1) % neighbors.num_neighbors];

                    // Check criteria of the middle neighbor
                    if (!hex_coord_is_valid(c2)) {
                        continue;
                    }
                    const Hex* hex2 = hex_at(c2.q, c2.r);
                    if (hex2->is_matched) {
                        continue;
                    }
                    if (in_cluster[c2.q][c2.r]) {
                        continue;
                    }
                    if (hex2->type != target_type) {
                        continue;
                    }

                    // Check prior neighbor
                    if (hex_coord_is_valid(c1) && hex_at(c1.q, c1.r)->type == target_type && in_cluster[c1.q][c1.r]) {
                        // SDL_Log("(%d,%d), Add (%d,%d)", c.q, c.r, c2.q, c2.r);
                        vector_push_back(dfs_stack, &c2);
                        in_cluster[c2.q][c2.r] = true;
                        continue;
                    }

                    // Check next neighbor
                    if (hex_coord_is_valid(c3) && hex_at(c3.q, c3.r)->type == target_type && in_cluster[c3.q][c3.r]) {
                        // SDL_Log("(%d,%d), Add (%d,%d)", c.q, c.r, c2.q, c2.r);
                        vector_push_back(dfs_stack, &c2);
                        in_cluster[c2.q][c2.r] = true;
                    }
                }
            }

            if (vector_size(hex_coords) >= 3) {
                goto done;
            }
        }
    }

done:
    vector_destroy(dfs_stack);

    if (vector_size(hex_coords) < 3) {
        vector_clear(hex_coords);
    }
    return vector_size(hex_coords);
}

size_t hex_find_one_bomb_cluster(Vector hex_coords) {
    // TODO
    return 0;
}

size_t hex_find_one_mmc_cluster(Vector hex_coords) {
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

Rectangle hex_bounding_box_of_coords(const HexCoord* coords, size_t num_coords) {
    Rectangle r = {
        .top_left = {LOGICAL_WINDOW_WIDTH, LOGICAL_WINDOW_HEIGHT},
        .bottom_right = {0,0},
    };
    for (size_t i = 0; i < num_coords; i++) {
        HexCoord c = coords[i];
        const Hex* hex = hex_at(c.q, c.r);
        r.top_left.x = MIN(r.top_left.x, hex->hex_point.x);
        r.top_left.y = MIN(r.top_left.y, hex->hex_point.y);
        r.bottom_right.x = MAX(r.bottom_right.x, hex->hex_point.x + HEX_WIDTH);
        r.bottom_right.y = MAX(r.bottom_right.y, hex->hex_point.y + HEX_HEIGHT);
    }
    r.width = r.bottom_right.x - r.top_left.x;
    r.height = r.bottom_right.y - r.top_left.y;
    return r;
}
