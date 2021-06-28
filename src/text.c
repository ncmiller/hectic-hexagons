#include "text.h"
#include "game_state.h"
#include <stdlib.h>

void text_init(Text* text) {
    memset(text, 0, sizeof(*text));
    text->needs_rendering = true;
}

char* text_buffer(Text* text) {
    text->needs_rendering = true;
    return text->buffer;
}

void text_set_font(Text* text, TTF_Font* font) {
    text->font = font;
    text->needs_rendering = true;
}

void text_set_position(Text* text, int x, int y) {
    text->position.x = x;
    text->position.y = y;
    text->needs_rendering = true;
}

void text_set_color(Text* text, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    text->color.r = r;
    text->color.g = g;
    text->color.b = b;
    text->color.a = a;
    text->needs_rendering = true;
}

static void render_texture(Text* text) {
    SDL_Surface* surface = TTF_RenderText_Blended(text->font, text->buffer, text->color);
    if (!surface) {
        SDL_Log("TTF_RenderText_Blended for %s failed, error: %s", text->buffer, TTF_GetError());
        return;
    }

    if (text->texture) {
        SDL_DestroyTexture(text->texture);
    }
    text->texture = SDL_CreateTextureFromSurface(g_state.renderer, surface);
    if (!text->texture) {
        SDL_Log("SDL_CreateTextureFromSurface for %s failed, error: %s", text->buffer, SDL_GetError());
    }

    text->width = surface->w;
    text->height = surface->h;

    SDL_FreeSurface(surface);
}

void text_draw(Text* text) {
    if (text->needs_rendering) {
        render_texture(text);
        text->needs_rendering = false;
    }

    if (text->texture) {
        SDL_Rect dest = {
            .x = text->position.x,
            .y = text->position.y,
            .w = text->width,
            .h = text->height,
        };
        SDL_RenderCopy(g_state.renderer, text->texture, NULL, &dest);
    }
}
