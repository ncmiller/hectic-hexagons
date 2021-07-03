#pragma once

#include <stdbool.h>
#include "input.h"
#include "game.h"
#include "graphics.h"

#define HEX_NUM_COLUMNS 10
#define HEX_NUM_ROWS 9
#define CURSOR_NUM_COLUMNS 9
#define CURSOR_NUM_ROWS 15

#if 0 // 1080p
#define LOGICAL_WINDOW_WIDTH 1920
#define LOGICAL_WINDOW_HEIGHT 1080
// Source png is 60 x 52, scaling up by 1.75
#define HEX_SOURCE_WIDTH 60
#define HEX_SOURCE_HEIGHT 52
#define HEX_WIDTH 105
#define HEX_HEIGHT 91
#define CURSOR_RADIUS 12
#define FONT_SIZE 24
#else // 720p
#define LOGICAL_WINDOW_WIDTH 1280
#define LOGICAL_WINDOW_HEIGHT 720
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
    HEX_TYPE_STAR_FLOWER,
    HEX_TYPE_BLACK_PEARL_UP,
    HEX_TYPE_BLACK_PEARL_DOWN,
    NUM_HEX_TYPES
} HexType;

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
    uint8_t q;
    uint8_t r;
} HexCoord;

typedef struct {
    // It's 7 because there are up to 6 surrounding hexes and we might count the one
    // in the middle too if it's neighbors of a starflower.
    HexCoord coords[7];
    size_t num_neighbors;
} HexNeighbors;

typedef struct {
    // We use a special coordinate system for the cursor (q is column, r is row).
    //
    // Cursor (q,r) == (0,0) is top left between the trio of hex coords (0,0), (1,0), (1,1).
    // As q increases it zig-zags left and right within the column until it reaches the bottom.
    // As r increases it jumps to the next hex trio intersection point.
    //
    // TODO - how to handle starflowers and black pearls?
    int q;
    int r;
} CursorCoord;

typedef struct {
    bool is_visible;
    CursorCoord coord;
    Point screen_point; // TODO - remove, precompute with lookup table
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
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;

    Statistics statistics;
    Constants constants;
    Input input;
    Game game;
    Graphics graphics;

    Text level_text;
    Text combos_text;
    Text score_text;
    Text fps_text;

    // On the last row, only the odd column hexes are valid. This is consistent
    // with the original Hexic HD board.
    Hex hexes[HEX_NUM_COLUMNS][HEX_NUM_ROWS];
    Cursor cursor;
} GameState;

extern GameState g_state;
