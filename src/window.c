#include "window.h"

#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>

static SDL_Renderer* _renderer;
static SDL_Window* _window;

static void print_version_info(void) {
    SDL_version sdl_version;
    SDL_version ttf_version;
    SDL_version image_version;
    SDL_version mixer_version;

    SDL_VERSION(&sdl_version);
    SDL_TTF_VERSION(&ttf_version);
    SDL_IMAGE_VERSION(&image_version);
    SDL_MIXER_VERSION(&mixer_version);

    SDL_Log("SDL version:       %d.%d.%d",
            sdl_version.major,
            sdl_version.minor,
            sdl_version.patch);
    SDL_Log("SDL_ttf version:   %d.%d.%d",
            ttf_version.major,
            ttf_version.minor,
            ttf_version.patch);
    SDL_Log("SDL_image version: %d.%d.%d",
            image_version.major,
            image_version.minor,
            image_version.patch);
    SDL_Log("SDL_mixer version: %d.%d.%d",
            mixer_version.major,
            mixer_version.minor,
            mixer_version.patch);
}

bool window_init(void) {
    print_version_info();

    if (0 != SDL_Init(SDL_INIT_EVERYTHING)) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    int flags = IMG_INIT_PNG;
    int initialized = IMG_Init(flags);
    if ((initialized & flags) != flags) {
        SDL_Log("IMG_Init failed init flags: 0x%08X", flags);
        return false;
    }

    if (TTF_Init() != 0) {
        SDL_Log("TTF_Init failed, error: %s", TTF_GetError());
        return false;
    }

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0) {
        SDL_Log("Mix_OpenAudio failed, error: %s", Mix_GetError());
        return false;
    }

    SDL_Log("SDL initialized\n");
    return true;
}

bool window_create(void) {
    _window = SDL_CreateWindow(
        "Hectic Hexagons",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        LOGICAL_WINDOW_WIDTH, LOGICAL_WINDOW_HEIGHT,
        0
    );

    if (_window == NULL) {
        SDL_Log("Could not create window: %s", SDL_GetError());
        window_close();
        return false;
    }

    _renderer = SDL_CreateRenderer(
            _window,
            -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (_renderer == NULL) {
        SDL_Log("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        window_close();
        return false;
    }

    SDL_RenderSetLogicalSize(_renderer, LOGICAL_WINDOW_WIDTH, LOGICAL_WINDOW_HEIGHT);
    return true;
}

void window_close(void) {
    if (_renderer) {
        SDL_DestroyRenderer(_renderer);
    }
    if (_window) {
        SDL_DestroyWindow(_window);
    }

    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

SDL_Renderer* window_renderer(void) {
    return _renderer;
}
