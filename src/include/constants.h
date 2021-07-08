#pragma once

#include "point.h"
#include <stdbool.h>

typedef struct {
    double hex_s; // hex radius
    double hex_h; // hex height
    double hex_w; // hex width
    Point board;  // Upper-left corner of board, in screen space
    double board_width;
    double board_height;

    // TODO - precompute hex screen points lookup table
    // TODO - precompute cursor screen points lookup table
} Constants;

bool constants_init(void);

extern Constants g_constants;
