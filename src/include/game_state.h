#pragma once
#include <stdbool.h>
#include "input.h"
#include "game.h"
#include "graphics.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define NUM_HEXES 3

typedef struct {
    float render_ave_ms;
    float update_ave_ms;
    Uint32 total_frames;
} Statistics;

typedef enum {
    HEX_ID_GREEN,
    HEX_ID_BLUE,
    HEX_ID_YELLOW,
    HEX_ID_MAGENTA,
    HEX_ID_PURPLE,
    HEX_ID_RED,
} HexID;

typedef struct {
    HexID hex_id;
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
    Hex hexes[NUM_HEXES];
} GameState;

// All game state data is stored in here.
extern GameState g_state;
