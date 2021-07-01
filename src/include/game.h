#pragma once

#include <SDL.h>
#include "geometry.h"

typedef struct {
    uint32_t level;
    uint32_t combos_remaining;
    uint32_t score;

    // Rotation
    // TODO - might want to replace this with a vector of animations
    bool rotation_in_progress;
    double degrees_to_rotate;
    int rotation_start_time;
} Game;

bool game_init(void);
void game_update(void);
