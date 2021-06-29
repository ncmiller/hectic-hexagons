#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include "text.h"

#define GRAPHICS_NUM_HEXES 3
#define GRAPHICS_TARGET_FPS 60

typedef enum {
    HEX_ID_GREEN,
    HEX_ID_BLUE,
    HEX_ID_YELLOW,
    HEX_ID_MAGENTA,
    HEX_ID_PURPLE,
    HEX_ID_RED,
} HexID;

typedef struct {
    HexID hex_id;
    Point hex_point; // upper-left corner
    double scale;
    Point rotation_point;
    double rotation_angle;
    double alpha;
} Hex;

typedef struct {
    SDL_Texture* hex_basic_texture;
    TTF_Font* font;
    Text level_text;
    Text combos_text;
    Text score_text;
    Text fps_text;
    Hex hexes[GRAPHICS_NUM_HEXES];
} Graphics;

bool graphics_init(void);
void graphics_update(void);
void graphics_flip(void);
