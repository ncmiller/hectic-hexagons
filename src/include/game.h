#pragma once

#include "text.h"
#include "vector.h"
#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_NUM_LEVELS 7

typedef struct {
    uint32_t score;
    uint64_t fade_start_time;
    Text text;
} LocalScore;

typedef struct {
    uint32_t level;
    uint32_t combos_remaining;
    uint32_t score;

    // Rotation
    bool rotation_in_progress;
    double degrees_to_rotate;
    uint64_t rotation_start_time;

    // Temporary storage for HexCoords, used during hex matching phase
    Vector hex_coords;

    // Temporary storage for LocalScores
    Vector local_scores;
} Game;

bool game_init(void);
void game_update(void);
