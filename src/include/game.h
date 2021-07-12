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
    uint64_t start_time;
    uint32_t score;
    double alpha; // range [0.0, 1.0]
    Point start_point;
    Point current_point;

    // Managed by graphics
    Text text;
} LocalScoreAnimation;

typedef struct {
    bool in_progress;
    uint64_t start_time;
    HexCoord flower_center;
} FlowerMatchAnimation;

typedef struct {
    bool in_progress;
    uint64_t start_time;
    HexCoord hex_coord;
} ClusterMatchAnimation;

typedef struct {
    uint32_t level;
    uint32_t combos_remaining;
    uint32_t score;

    // Rotation animation
    bool rotation_in_progress;
    double degrees_to_rotate;
    uint64_t rotation_start_time;

    Vector flower_match_animations; // FlowerMatchAnimation
    Vector local_score_animations; // LocalScoreAnimation
    Vector cluster_match_animations; // ClusterMatchAnimation
} Game;

bool game_init(void);
void game_update(void);
