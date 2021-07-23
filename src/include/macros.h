#pragma once

#include "test_boards.h"
#include "game_state.h"
#include "cursor.h"
#include <assert.h>

#define MAX(a, b) \
    ({ \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a > _b ? _a : _b; \
    })

#define MIN(a, b) \
    ({ \
        __typeof__(a) _a = (a); \
        __typeof__(b) _b = (b); \
        _a < _b ? _a : _b; \
    })

#define ASSERT(x) \
    if (!(x)) { \
        SDL_Log( \
            "\n\n----\n" \
            "Assertion failed\n  %s:%d\n" \
            "----", __FILE__, __LINE__); \
        test_boards_print_current(); \
        cursor_print(); \
        g_state.suspend_game = true; \
    }
