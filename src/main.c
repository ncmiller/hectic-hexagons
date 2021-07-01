#include <SDL.h>
#include <SDL_image.h>
#include <stdbool.h>

#include "game_state.h"

GameState g_state = {0};

static void sdl_init(void) {
    if (0 != SDL_Init(SDL_INIT_EVERYTHING)) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        exit(1);
    }

    int flags = IMG_INIT_PNG;
    int initialized = IMG_Init(flags);
    if ((initialized & flags) != flags) {
        SDL_Log("IMG_Init failed init flags: 0x%08X", flags);
        exit(1);
    }

    if (TTF_Init() != 0) {
        SDL_Log("TTF_Init failed, error: %s'\n", TTF_GetError());
        exit(1);
    }

    SDL_Log("SDL initialized\n");
}

static void sdl_close(void) {
    if (g_state.renderer) {
        SDL_DestroyRenderer(g_state.renderer);
    }
    if (g_state.window) {
        SDL_DestroyWindow(g_state.window);
    }

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

static void sdl_create_window(void) {
    g_state.window = SDL_CreateWindow(
        "Hectic Hexagons",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        0
    );

    if (g_state.window == NULL) {
        SDL_Log("Could not create window: %s", SDL_GetError());
        sdl_close();
        exit(1);
    }

    g_state.renderer = SDL_CreateRenderer(
            g_state.window,
            -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (g_state.renderer == NULL) {
        SDL_Log("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        sdl_close();
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    sdl_init();
    sdl_create_window();

    bool init_success = true;
    init_success &= input_init();
    init_success &= game_init();
    init_success &= graphics_init();
    if (!init_success) {
        sdl_close();
        return 1;
    }

    g_state.running = true;
    while (g_state.running) {
        uint32_t start = SDL_GetTicks();
        input_update();
        game_update();
        graphics_update();
        uint32_t update_diff = SDL_GetTicks() - start;
        graphics_flip();
        uint32_t render_diff = SDL_GetTicks() - start;

        Statistics* stats = &g_state.statistics;
        float smoothing = 0.9f;
        stats->update_ave_ms = (stats->update_ave_ms * smoothing) + ((float)update_diff * (1.0f - smoothing));
        stats->render_ave_ms = (stats->render_ave_ms * smoothing) + ((float)render_diff * (1.0f - smoothing));
        stats->total_frames++;
    }

    sdl_close();
    return 0;
}
