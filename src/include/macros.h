#pragma once

#include "test_boards.h"
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
        test_boards_print_current(); \
        assert((x)); \
    }
