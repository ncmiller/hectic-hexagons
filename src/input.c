#include "input.h"
#include "game_state.h"
#include <SDL.h>

bool input_init(void) {
    return true;
}

void input_update(void) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            g_state.running = false;
        } else if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_x) {
                g_state.input.rotate_cw = true;
            } else if (e.key.keysym.sym == SDLK_z) {
                g_state.input.rotate_ccw = true;
            } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                g_state.running = false;
            }
        }
    }
}
