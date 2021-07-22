#include "input.h"
#include "game_state.h"
#include "graphics.h"
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
            } else if (e.key.keysym.sym == SDLK_UP) {
                g_state.input.up = true;
            } else if (e.key.keysym.sym == SDLK_DOWN) {
                g_state.input.down = true;
            } else if (e.key.keysym.sym == SDLK_LEFT) {
                g_state.input.left = true;
            } else if (e.key.keysym.sym == SDLK_RIGHT) {
                g_state.input.right = true;
            } else if (e.key.keysym.sym == SDLK_p) {
                g_state.input.print_board = true;
            } else if (e.key.keysym.sym == SDLK_SPACE) {
                g_state.suspend_game = !g_state.suspend_game;
                SDL_Log("%s game", g_state.suspend_game ? "Suspending" : "Resuming");
            } else if (e.key.keysym.sym == SDLK_l) {
                g_state.slow_mode = !g_state.slow_mode;
                SDL_Log("%s mode", g_state.slow_mode ? "Slow" : "Normal");
            }
        }
    }
}
