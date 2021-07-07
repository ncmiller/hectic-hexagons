#pragma once

#include <SDL.h>
#include <stdbool.h>

#if 0 // 1080p
#define LOGICAL_WINDOW_WIDTH 1920
#define LOGICAL_WINDOW_HEIGHT 1080
#else
#define LOGICAL_WINDOW_WIDTH 1280
#define LOGICAL_WINDOW_HEIGHT 720
#endif

// Returns false if init fails
bool window_init(void);

// Create window and renderer. Returns false on error.
bool window_create(void);

// Close the window, end program
void window_close(void);

SDL_Renderer* window_renderer(void);
