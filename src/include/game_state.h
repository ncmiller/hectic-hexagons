#pragma once

#include <stdbool.h>
#include "input.h"
#include "game.h"
#include "graphics.h"

#define HEX_NUM_COLUMNS 10
#define HEX_NUM_ROWS 9

#if 0 // 1080p
// Source png is 60 x 52, scaling up by 1.75
#define HEX_SOURCE_WIDTH 60
#define HEX_SOURCE_HEIGHT 52
#define HEX_WIDTH 105
#define HEX_HEIGHT 91
#define CURSOR_RADIUS 12
#define FONT_SIZE 24
#else // 720p
#define HEX_SOURCE_WIDTH 60
#define HEX_SOURCE_HEIGHT 52
#define HEX_WIDTH 60
#define HEX_HEIGHT 52
#define CURSOR_RADIUS 8
#define FONT_SIZE 20
#endif

typedef struct {
    float render_ave_ms;
    float update_ave_ms;
    uint32_t total_frames;
} Statistics;

typedef enum {
    HEX_TYPE_GREEN,
    HEX_TYPE_BLUE,
    HEX_TYPE_YELLOW,
    HEX_TYPE_MAGENTA,
    HEX_TYPE_PURPLE,
    HEX_TYPE_RED,
    // TODO multipliers
    // TODO bombs
    HEX_TYPE_STARFLOWER,
    HEX_TYPE_BLACK_PEARL_UP,
    HEX_TYPE_BLACK_PEARL_DOWN,
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
    bool is_valid;
    HexType type;
    Point hex_point; // TODO - remove, precompute
    bool is_rotating;
    double scale;
    double rotation_angle;
    double alpha;
} Hex;

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

typedef enum {
    CURSOR_POS_LEFT,
    CURSOR_POS_RIGHT,
    CURSOR_POS_ON,
} CursorPos;

typedef struct {
    // Cursor is assocatied with a particular hex coordinate.
    // The actual location of the cursor will either be to the
    // left of, on, or to the right of the hex.
    HexCoord hex_anchor;
    CursorPos position;
    Point screen_point;
    bool is_visible;
} Cursor;

typedef struct {
    double hex_s; // hex radius
    double hex_h; // hex height
    double hex_w; // hex width
    Point board;  // Upper-left corner of board, in screen space
    double board_width;
    double board_height;

    // TODO - precompute hex screen points lookup table
    // TODO - precompute cursor screen points lookup table
} Constants;

typedef struct {
    bool running;

    Statistics statistics;
    Constants constants;
    Input input;
    Game game;
    Cursor cursor;

    // On the last row, only the odd column hexes are valid. This is consistent
    // with the original Hexic HD board.
    Hex hexes[HEX_NUM_COLUMNS][HEX_NUM_ROWS];
} GameState;

extern GameState g_state;
