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

    // On the last row, only the odd column hexes are valid. This is consistent
    // with the original Hexic HD board.
    Hex hexes[HEX_NUM_COLUMNS][HEX_NUM_ROWS];
} GameState;

extern GameState g_state;
