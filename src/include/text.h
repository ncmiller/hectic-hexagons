#pragma once
#include <stdbool.h>
#include <SDL_ttf.h>
#include "geometry.h"

#define TEXT_MAX_LEN 128

typedef struct {
    char buffer[TEXT_MAX_LEN + 1];
    Point point;
    SDL_Color color;
    TTF_Font* font;
    bool needs_rendering;
    SDL_Texture* texture;
    int height;
    int width;
    SDL_Rect texture_dest;
} Text;

void text_init(Text*);
char* text_buffer(Text*);
void text_set_font(Text*, TTF_Font*);
void text_set_point(Text*, int x, int y);
void text_set_color(Text*, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void text_draw(Text*);
