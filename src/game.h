#pragma once

#include <SDL.h>

typedef struct {
    Uint32 level;
    Uint32 combos_remaining;
    Uint32 score;
} Game;

bool game_init(void);
void game_update(void);
