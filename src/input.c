#include "input.h"
#include "game_state.h"
#include <SDL.h>

bool input_init(void) {
    return true;
}

void input_update(void) {
    g_state.input.mouse_left_click = false;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            g_state.running = false;
        }
        if (e.type == SDL_KEYDOWN) {
            g_state.running = false;
        }
        if (e.type == SDL_MOUSEBUTTONUP) {
            g_state.input.mouse_left_click = true;
        }
        if (e.type == SDL_MOUSEMOTION) {
            SDL_GetMouseState(&g_state.input.mouse_position.x, &g_state.input.mouse_position.y);
        }
    }
}
