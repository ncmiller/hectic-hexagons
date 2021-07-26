#pragma once

#include "point.h"
#include <stdbool.h>

#define HEX_NUM_COLUMNS 10
#define HEX_NUM_ROWS 9

typedef struct {
    double hex_s; // hex radius
    double hex_h; // hex height
    double hex_w; // hex width
    Point board;  // Upper-left corner of board, in screen space
    double board_width;
    double board_height;

    Point hex_to_screen[HEX_NUM_COLUMNS][HEX_NUM_ROWS];
    Point hex_spawn_point[HEX_NUM_COLUMNS];

    // TODO - precompute cursor screen points lookup table
} Constants;

bool constants_init(void);

Point transform_hex_to_screen(int q, int r);

extern Constants g_constants;
