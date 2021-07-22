#pragma once

#include <stdbool.h>

// X: rotate clockwise
// Z: rotate counter-clockwise
// Arrow keys: move cursor
// ESC: exit game
// Spacebar: suspend game
// P: Print current board to console
// L: slow mode (5 Hz)

typedef struct {
    // Set on keypress, cleared by game when read
    bool rotate_cw;
    bool rotate_ccw;
    bool up;
    bool down;
    bool left;
    bool right;
    bool print_board;
} Input;

bool input_init(void);
void input_update(void);
