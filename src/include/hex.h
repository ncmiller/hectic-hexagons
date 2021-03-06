#pragma once

#include "point.h"
#include "vector.h"
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#if 0 // 1080p
// Source png is 60 x 52, scaling up by 1.75
#define HEX_WIDTH 105
#define HEX_HEIGHT 91
#else // 720p
#define HEX_WIDTH 60
#define HEX_HEIGHT 52
#endif

// Bitmask to select specific hex neighbors in the bottom 6 bits.
// Bit index corresponds to HexNeighborID (e.g. bit 0 is top, bit 1 is top right, etc).
#define ALL_NEIGHBORS              0x3F
#define BLACK_PEARL_UP_NEIGHBORS   0x15
#define BLACK_PEARL_DOWN_NEIGHBORS 0x2A
#define TRIO_LEFT_NEIGHBORS        0x30
#define TRIO_RIGHT_NEIGHBORS       0x06

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
    NUM_HEX_TYPES,
    HEX_TYPE_INVALID = -1,
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
    bool in_progress;
    uint32_t start_time;
    Point flower_center;
    bool is_center;
} FlowerMatchAnimation;

typedef struct {
    bool in_progress;
    uint32_t start_time;
} ClusterMatchAnimation;

typedef struct {
    bool is_valid; // TODO - do we even need this?
    bool is_dead;

    HexType type;

    // Updated by gravity system
    Point hex_point;
    double velocity;
    uint32_t gravity_start_time;
    bool is_stationary; // not falling, candidate for matching

    // Animations
    FlowerMatchAnimation flower_match_animation;
    ClusterMatchAnimation cluster_match_animation;
    bool is_rotating;

    // Transformations, modified by animations
    double scale;
    double alpha; // range [0.0, 1.0]
    double rotation_angle; // degrees

    // True if the hex is combo'd in any way.
    bool is_matched;

    // True if the hex is part of a flower match
    bool is_flower_matched;
} Hex;

// Get coordinate of specific neighbor of hex at (q,r)
HexCoord hex_neighbor_coord(int q, int r, HexNeighborID neighbor_id);

// Returns hex neighbors of hex (q, r) in clockwise order.
// Neighbor is only added if the corresponding HexNeighborID bit is set in the mask.
// Note that some of these may be invalid or outside of bounds.
// Caller should check them with hex_coord_is_valid(q, r).
void hex_neighbors(int q, int r, HexNeighbors* neighbors, uint8_t mask);

// Spawn a random hex in column q.
// Creates a new hex and pushes it to the top of the column stack.
// Initial position is above the view port.
Hex* hex_spawn(int q);

// Generate a random, level-appropriate hex.
HexType hex_random_type(void);

// Generate a random hex if hex is in the mask
HexType hex_random_type_with_mask(uint32_t mask);

Hex* hex_at(int q, int r);

// Returns true if the hex at (q,r) has a trio match.
// If n1 and n2 are non-NULL and trio match found, populate with neighbor coords.
bool hex_has_cluster_match(int q, int r, HexCoord* n1, HexCoord* n2, bool require_stationary);

// Returns true if the hex at (q,r) has a flower match (all neighbors are the same type).
bool hex_has_flower_match(int q, int r, bool require_stationary);

// Find a single flower and add coordinates of center and neighbors to hex_coords.
// The flower center will be in index 0, and the 6 neighbors will start at index 1.
//
// To be considered, a hex must have is_matched == false.
size_t hex_find_one_flower(Vector hex_coords);

// Finds a single simple cluster (3, 4, or 5 of same hex type) and adds the coordinates of each
// hex in the cluster to hex_coords.
//
// Multipliers can cluster without having to be the same color.
//
// To be considered, a hex must have is_matched == false.
size_t hex_find_one_simple_cluster(Vector hex_coords);

// Finds bomb clusters (mix of a basic, bomb, and multipliers of a single color) and
// adds the coordinates of each hex in the cluster to hex_coords.
//
// To be considered, a hex must have is_matched == false.
size_t hex_find_one_bomb_cluster(Vector hex_coords);

// Finds a single MMC cluster (mix of a basic and multipliers of a single color) and
// adds the coordinates of each hex in the cluster to hex_coords.
//
// To be considered, a hex must have is_matched == false.
size_t hex_find_one_mmc_cluster(Vector hex_coords);

bool hex_coord_is_valid(HexCoord coord);

// Given several coordinates, get the bounding box, in screen space
Rectangle hex_bounding_box_of_coords(const HexCoord* coords, size_t num_coords);

bool hex_is_basic(const Hex* hex);
bool hex_is_multiplier(const Hex* hex);
bool hex_is_starflower(const Hex* hex);
bool hex_is_bomb(const Hex* hex);
bool hex_is_black_pearl(const Hex* hex);

void hex_print(const Hex* hex);
Point transform_hex_to_screen(int q, int r);

int hex_row_to_stack_index(int row);
int hex_stack_index_to_row(int stack_index);

bool hex_is_animating(const Hex* hex);

// Returns true if all hexes are stationary, not moving or animating.
bool hex_all_stationary_no_animation(void);
