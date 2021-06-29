#pragma once

#include <stdbool.h>
#include "geometry.h"

typedef struct {
    Point mouse_point;
    bool mouse_left_click;
    bool rotate_cw;
    bool rotate_ccw;
} Input;

bool input_init(void);
void input_update(void);
