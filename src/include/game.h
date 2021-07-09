#pragma once

#include "text.h"
#include "vector.h"
#include "hex.h"
#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    bool in_progress;
    uint64_t start_time;
    uint32_t score;
    Text text;
} LocalScoreAnimation;

typedef struct {
    bool in_progress;
    uint64_t start_time;
    HexCoord flower_center;
} FlowerMatchAnimation;

typedef struct {
    uint32_t level;
    uint32_t combos_remaining;
    uint32_t score;

    // Rotation animation
    bool rotation_in_progress;
    double degrees_to_rotate;
    uint64_t rotation_start_time;

    Vector hex_coords; // HexCoord
    Vector flower_match_animations; // FlowerMatchAnimation
    Vector local_score_animations; // LocalScoreAnimation
} Game;

bool game_init(void);
void game_update(void);
