#pragma once

#include <stdbool.h>

typedef struct {
    // Set on keypress, cleared by game when read
    bool rotate_cw;
    bool rotate_ccw;
    bool up;
    bool down;
    bool left;
    bool right;
} Input;

bool input_init(void);
void input_update(void);
