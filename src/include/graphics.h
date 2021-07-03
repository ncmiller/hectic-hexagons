#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include "text.h"

typedef struct {
    SDL_Texture* hex_basic_texture;
    TTF_Font* font;
} Graphics;

bool graphics_init(void);
void graphics_update(void);
void graphics_flip(void);
