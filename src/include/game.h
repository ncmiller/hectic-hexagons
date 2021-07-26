#pragma once

#include "text.h"
#include "vector.h"
#include "hex.h"
#include <SDL.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    // Managed by game
    bool in_progress;
    uint32_t start_time;
    uint32_t score;
    double alpha; // range [0.0, 1.0]
    Point start_point;
    Point current_point;

    // Managed by graphics
    Text text;
} LocalScoreAnimation;

typedef struct {
    bool in_progress;
    uint32_t start_time;
    Point rotation_center;
    bool is_trio_rotation;
    double degrees_to_rotate;
    uint32_t rotation_count;
} RotationAnimation;

typedef struct {
    uint32_t seed;
    uint32_t level;
    uint32_t combos_remaining;
    uint32_t score;
    double gravity;

    // Each column is Vector containing type Hex.
    // The vector represents the stack of hexes on the board
    // (i.e. vector index 0 is the bottom of the stack/board).
    Vector hexes[HEX_NUM_COLUMNS];

    RotationAnimation rotation_animation;
    Vector local_score_animations; // contains LocalScoreAnimation
} Game;

bool game_init(void);
bool game_update(void);
