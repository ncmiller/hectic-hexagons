#include <SDL.h>

int main(int argc, char* argv[]) {
    const Uint32 subsystems = SDL_INIT_VIDEO | SDL_INIT_AUDIO;
    if (0 != SDL_Init(subsystems)) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return 1;
    }

    SDL_Log("SDL initialized\n");
    SDL_Quit();

    return 0;
}
