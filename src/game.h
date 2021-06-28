#pragma once

#include <SDL.h>

typedef struct {
    bool follow_mouse;
    Uint32 level;
    Uint32 combos_remaining;
    Uint32 score;
    Position hex_position;
} Game;

bool game_init(void);
void game_update(void);
