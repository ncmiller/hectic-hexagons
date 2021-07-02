#pragma once
#include <stdbool.h>
#include "input.h"
#include "game.h"
#include "graphics.h"

#define HEX_NUM_COLUMNS 10
#define HEX_NUM_ROWS 9

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
    Point hex_point; // upper-left corner
    double scale;
    Point rotation_point;
    double rotation_angle;
    double alpha;
} Hex;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    Statistics statistics;
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
} GameState;

extern GameState g_state;
