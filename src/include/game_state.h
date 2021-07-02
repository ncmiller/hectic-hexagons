#pragma once
#include <stdbool.h>
#include "input.h"
#include "game.h"
#include "graphics.h"

#define HEX_NUM_COLUMNS 10
#define HEX_NUM_ROWS 9
#define CURSOR_NUM_COLUMNS 9
#define CURSOR_NUM_ROWS 15

// Source png is 60 x 52, scaling up by 1.75
#define HEX_SOURCE_WIDTH 60
#define HEX_SOURCE_HEIGHT 52
#define HEX_WIDTH 105
#define HEX_HEIGHT 91

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
    NUM_HEX_TYPES
} HexType;

typedef struct {
    bool is_valid;
    HexType hex_type;
    Point hex_point; // TODO - remove, precompute
    double scale;
    double rotation_angle;
    double alpha;
} Hex;

typedef struct {
    bool is_visible;

    // We use a special coordinate system for the cursor (q is column, r is row).
    //
    // Cursor (q,r) == (0,0) is top left between the trio of hex coords (0,0), (1,0), (1,1).
    // As q increases it zig-zags left and right within the column until it reaches the bottom.
    // As r increases it jumps to the next hex trio intersection point.
    //
    // TODO - how to handle starflowers and black pearls?
    int q;
    int r;

    Point screen_point; // TODO - remove, precompute
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

    // We use the "even-q" indexing scheme described on this page:
    // https://www.redblobgames.com/grids/hexagons
    //
    // On the last row, only the odd column hexes are valid. This is consistent
    // with the original Hexic HD board.
    //
    // For even columns, the last row does not have a valid hex. Only odd columns
    // have valid hexes.
    Hex hexes[HEX_NUM_COLUMNS][HEX_NUM_ROWS];
    Cursor cursor;
} GameState;

extern GameState g_state;
