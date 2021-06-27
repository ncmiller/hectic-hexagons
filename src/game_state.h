#pragma once
#include <stdbool.h>
#include "input.h"
#include "game.h"
#include "graphics.h"

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;
    Input input;
    Game game;
    Graphics graphics;
} GameState;

// All game state data is stored in here.
extern GameState g_state;
