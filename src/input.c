#include "input.h"
#include "game_state.h"
#include <SDL.h>

bool input_init(void) {
    return true;
}

void input_update(void) {
    g_state.input.mouse_left_click = false;
    g_state.input.rotate_cw = false;
    g_state.input.rotate_ccw = false;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            g_state.running = false;
        } else if (e.type == SDL_MOUSEBUTTONUP) {
            g_state.input.mouse_left_click = true;
        } else if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_x) {
                g_state.input.rotate_cw = true;
            } else if (e.key.keysym.sym == SDLK_z) {
                g_state.input.rotate_ccw = true;
            } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                g_state.running = false;
            }
        } else if (e.type == SDL_MOUSEMOTION) {
            SDL_GetMouseState(&g_state.input.mouse_point.x, &g_state.input.mouse_point.y);
        }
    }
}
