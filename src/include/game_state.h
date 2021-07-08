#pragma once

#include <stdbool.h>
#include "input.h"
#include "game.h"
#include "cursor.h"
#include "hex.h"

typedef struct {
    bool running;
    Input input;
    Game game;
    Cursor cursor;
    Hex hexes[HEX_NUM_COLUMNS][HEX_NUM_ROWS];
} GameState;

extern GameState g_state;
