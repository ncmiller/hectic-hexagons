#pragma once

#include <SDL.h>
#include <SDL_ttf.h>
#include <stdbool.h>
#include "text.h"

bool graphics_init(void);
void graphics_update(void);
void graphics_flip(void);
