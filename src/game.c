#include "game_state.h"

bool game_init(void) {
    return true;
}

void game_update(void) {
    if (g_state.input.mouse_left_click) {
        g_state.game.follow_mouse = !g_state.game.follow_mouse;
    }

    if (g_state.game.follow_mouse) {
        g_state.game.hex_position = g_state.input.mouse_position;
    }
}
