#pragma once

#include "point.h"
#include "game.h"
#include <stdbool.h>
#include <stdlib.h>

#if 0 // 1080p
// Source png is 60 x 52, scaling up by 1.75
#define HEX_WIDTH 105
#define HEX_HEIGHT 91
#else // 720p
#define HEX_WIDTH 60
#define HEX_HEIGHT 52
#endif

#define HEX_NUM_COLUMNS 10
#define HEX_NUM_ROWS 9

// Minus 5 because in the last row, only 5 of 10 hexes are valid
#define NUM_TOTAL_HEXES (HEX_NUM_COLUMNS * HEX_NUM_ROWS - 5)

// Bitmask to select specific hex neighbors in the bottom 6 bits.
// Bit index corresponds to HexNeighborID (e.g. bit 0 is top, bit 1 is top right, etc).
#define ALL_NEIGHBORS              0x3F
#define BLACK_PEARL_UP_NEIGHBORS   0x15
#define BLACK_PEARL_DOWN_NEIGHBORS 0x2A
#define TRIO_LEFT_NEIGHBORS        0x30
#define TRIO_RIGHT_NEIGHBORS       0x06

// Bitmasks to select specific hex types to spawn, based on level.
// Bit index corresponds to HexType (e.g. bit 0 is green, bit 1 is blue, etc).
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

typedef enum {
    HEX_TYPE_GREEN,
    HEX_TYPE_BLUE,
    HEX_TYPE_YELLOW,
    HEX_TYPE_MAGENTA,
    HEX_TYPE_PURPLE,
    HEX_TYPE_RED,
    HEX_TYPE_STARFLOWER,
    HEX_TYPE_BLACK_PEARL_UP,
    HEX_TYPE_BLACK_PEARL_DOWN,
    // TODO multipliers
    // TODO bombs
    NUM_HEX_TYPES
} HexType;

typedef enum {
    HEX_NEIGHBOR_TOP,
    HEX_NEIGHBOR_TOP_RIGHT,
    HEX_NEIGHBOR_BOTTOM_RIGHT,
    HEX_NEIGHBOR_BOTTOM,
    HEX_NEIGHBOR_BOTTOM_LEFT,
    HEX_NEIGHBOR_TOP_LEFT,
    MAX_NUM_HEX_NEIGHBORS,
} HexNeighborID;

typedef struct {
    // We use the "even-q" indexing scheme described on this page:
    // https://www.redblobgames.com/grids/hexagons
    int q;
    int r;
} HexCoord;

typedef struct {
    HexCoord coords[MAX_NUM_HEX_NEIGHBORS];
    size_t num_neighbors;
} HexNeighbors;

typedef struct {
    bool is_valid;
    HexType type;
    Point hex_point; // TODO - remove, precompute
    bool is_rotating;
    double scale;
    double rotation_angle;
    double alpha;
    bool is_falling;
    bool is_matched;
} Hex;

// Get coordinate of specific neighbor of hex at (q,r)
HexCoord hex_neighbor_coord(int q, int r, HexNeighborID neighbor_id);

// Returns hex neighbors of hex (q, r) in clockwise order.
// Neighbor is only added if the corresponding HexNeighborID bit is set in the mask.
// Note that some of these may be invalid or outside of bounds.
// Caller should check them with hex_coord_is_valid(q, r).
void hex_neighbors(int q, int r, HexNeighbors* neighbors, uint8_t mask);

// Spawn a random hex at (q,r)
void hex_spawn(int q, int r);

// Generate a random, level-appropriate hex.
HexType hex_random_type(void);

// Generate a random hex if hex is in the mask
HexType hex_random_type_with_mask(uint32_t mask);

Hex* hex_at(int q, int r);

// Returns true if the hex at (q,r) has a 3-, 4-, or 5-cluster match with neighbors.
bool hex_has_cluster_match(int q, int r);

// Returns true if the hex at (q,r) has a flower match (all neighbors are the same type).
bool hex_has_flower_match(int q, int r);

// Find a single flower and add coordinates of center and neighbors to hex_coords.
// The flower center will be in index 0, and the 6 neighbors will start at index 1.
//
// To be considered, a hex must have is_matched == false.
size_t hex_find_one_flower(Vector* hex_coords);

// Finds a single simple cluster (3, 4, or 5 of same hex type) and adds the coordinates of each
// hex in the cluster to hex_coords.
//
// Multipliers can cluster without having to be the same color.
//
// To be considered, a hex must have is_matched == false.
size_t hex_find_one_simple_cluster(Vector* hex_coords);

// Finds bomb clusters (mix of a basic, bomb, and multipliers of a single color) and
// adds the coordinates of each hex in the cluster to hex_coords.
//
// To be considered, a hex must have is_matched == false.
size_t hex_find_one_bomb_cluster(Vector* hex_coords);

// Finds a single MMC cluster (mix of a basic and multipliers of a single color) and
// adds the coordinates of each hex in the cluster to hex_coords.
//
// To be considered, a hex must have is_matched == false.
size_t hex_find_one_mmc_cluster(Vector* hex_coords);

bool hex_is_basic(const Hex* hex);
bool hex_is_multiplier(const Hex* hex);
bool hex_is_starflower(const Hex* hex);
bool hex_is_bomb(const Hex* hex);
bool hex_is_black_pearl(const Hex* hex);

void hex_clear_is_matched(Hex*);

typedef void (*HexFn)(Hex*);

// Calls fn on every hex in g_state.hexes.
void hex_for_each(HexFn fn);
