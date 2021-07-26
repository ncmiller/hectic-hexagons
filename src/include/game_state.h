#pragma once

#include <stdbool.h>
#include "input.h"
#include "game.h"
#include "cursor.h"
#include "hex.h"

typedef struct {
    uint32_t frame_count;
    uint32_t slow_mode_throttle;
    bool suspend_game;
    bool slow_mode;
    bool running;
    Input input;
    Game game;
    Cursor cursor;

    // Backing buffer for bump_allocator. Reset on each game loop iteration.
    uint8_t temporary_allocations[0x10000];
} GameState;

extern GameState g_state;
