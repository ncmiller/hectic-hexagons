#include "graphics.h"
#include "game_state.h"
#include <SDL_image.h>

#define HEX_WIDTH 60
#define HEX_HEIGHT 52

static SDL_Texture* load_texture(const char* path) {
    SDL_Surface* surface = IMG_Load(path);
    if (surface == NULL) {
        SDL_Log("IMG_Load(%s) failed: %s", path, SDL_GetError());
        return NULL;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(g_state.renderer, surface);
    if (texture == NULL) {
        SDL_Log("SDL_CreateTextureFromSurface(%s) failed: %s", path, SDL_GetError());
        return NULL;
    }

    SDL_FreeSurface(surface);
    return texture;
}

static bool load_all_media(void) {
    bool all_loaded = true;

    g_state.graphics.hex_basic_texture = load_texture("media/hex_basic.png");
    all_loaded &= (g_state.graphics.hex_basic_texture != NULL);

    if (!all_loaded) {
        SDL_Log("Failed to load media. Exiting.");
        return false;
    }
    return true;
}

bool graphics_init(void) {
    return load_all_media();
}

void graphics_draw_hex_at(HexID id, int x, int y) {
    SDL_Rect src = {
        .x = id * HEX_WIDTH,
        .y = 0,
        .w = HEX_WIDTH,
        .h = HEX_HEIGHT,
    };

    SDL_Rect dest = {
        .x = x,
        .y = y,
        .w = HEX_WIDTH,
        .h = HEX_HEIGHT
    };

    bool centered = true;
    if (centered) {
        dest.x -= HEX_WIDTH / 2;
        dest.y -= HEX_HEIGHT / 2;
    }
    SDL_RenderCopy(g_state.renderer, g_state.graphics.hex_basic_texture, &src, &dest);
}

void graphics_update(void) {
    SDL_RenderClear(g_state.renderer);
    SDL_SetRenderDrawColor(g_state.renderer, 0x44, 0x44, 0x44, 0xFF);
    graphics_draw_hex_at(HEX_ID_GREEN, 100, 100);
    graphics_draw_hex_at(HEX_ID_RED, 100 + HEX_WIDTH, 100);
    graphics_draw_hex_at(HEX_ID_MAGENTA, 100 + 2 * HEX_WIDTH, 100);
    graphics_draw_hex_at(HEX_ID_PURPLE, 100 + 3 * HEX_WIDTH, 100);
    graphics_draw_hex_at(HEX_ID_YELLOW, 100 + 4 * HEX_WIDTH, 100);
    graphics_draw_hex_at(HEX_ID_BLUE, 100 + 5 * HEX_WIDTH, 100);
    graphics_draw_hex_at(HEX_ID_BLUE, g_state.game.hex_position.x, g_state.game.hex_position.y);
    SDL_RenderPresent(g_state.renderer);
}
