#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include "text.h"

typedef struct {
    SDL_Texture* hex_basic_texture;
    TTF_Font* font;
    Text level_text;
    Text combos_text;
    Text score_text;
    Text fps_text;
} Graphics;

bool graphics_init(void);
void graphics_update(void);
void graphics_flip(void);
