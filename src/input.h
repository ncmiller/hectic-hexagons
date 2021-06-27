#pragma once

#include <stdbool.h>

typedef struct {
    int x;
    int y;
} Position;

typedef struct {
    Position mouse_position;
    bool mouse_left_click;
} Input;

bool input_init(void);
void input_update(void);
