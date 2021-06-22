#include <SDL.h>
#include <stdbool.h>

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

static SDL_Window* g_window;

int main(int argc, char* argv[]) {
    const Uint32 subsystems = SDL_INIT_VIDEO | SDL_INIT_AUDIO;
    if (0 != SDL_Init(subsystems)) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }
    SDL_Log("SDL initialized\n");

    g_window = SDL_CreateWindow(
        "Hectic Hexagons",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE
    );
    if (g_window == NULL) {
        SDL_Log("Could not create window: %s", SDL_GetError());
        return 1;
    }

    SDL_Event e;
    bool quit = false;
    while (!quit) {
        while (SDL_PollEvent(&e)){
            if (e.type == SDL_QUIT){
                quit = true;
            }
            if (e.type == SDL_KEYDOWN){
                quit = true;
            }
            if (e.type == SDL_MOUSEBUTTONDOWN){
                quit = true;
            }
        }
    }

    SDL_DestroyWindow(g_window);
    SDL_Quit();

    return 0;
}
