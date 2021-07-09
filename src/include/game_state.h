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
    // Backing buffer for bump_allocator. Reset on each game loop iteration.
    uint8_t temporary_allocations[32768];
} GameState;

extern GameState g_state;
