#pragma once

#include <stdbool.h>

typedef struct {
    // Set on keypress, cleared by game when read
    bool rotate_cw;
    bool rotate_ccw;
} Input;

bool input_init(void);
void input_update(void);
