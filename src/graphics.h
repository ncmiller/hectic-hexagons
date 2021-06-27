#pragma once

#include <SDL.h>
#include <stdbool.h>

typedef struct {
    SDL_Texture* hex_basic_texture;
} Graphics;

typedef enum {
    HEX_ID_GREEN,
    HEX_ID_BLUE,
    HEX_ID_YELLOW,
    HEX_ID_MAGENTA,
    HEX_ID_PURPLE,
    HEX_ID_RED,
} HexID;

bool graphics_init(void);
void graphics_draw_hex_at(HexID id, int x, int y);
void graphics_update(void);
