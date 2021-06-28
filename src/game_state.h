#pragma once
#include <stdbool.h>
#include "input.h"
#include "game.h"
#include "graphics.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

typedef struct {
    float render_ave_ms;
    float update_ave_ms;
    Uint32 total_frames;
} Statistics;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    Statistics statistics;
    Input input;
    Game game;
    Graphics graphics;
} GameState;

// All game state data is stored in here.
extern GameState g_state;
