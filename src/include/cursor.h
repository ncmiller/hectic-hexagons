#pragma once

#include "hex.h"
#include "point.h"
#include <stdbool.h>

typedef enum {
    CURSOR_POS_LEFT,
    CURSOR_POS_RIGHT,
    CURSOR_POS_ON,
} CursorPos;

typedef struct {
    // Cursor is assocatied with a particular hex coordinate.
    // The actual location of the cursor will either be to the
    // left of, on, or to the right of the hex.
    HexCoord hex_anchor;
    CursorPos position;
    Point screen_point;
    bool is_visible;
} Cursor;

void cursor_init(Cursor* cursor);

void cursor_up(Cursor* cursor);
void cursor_down(Cursor* cursor);
void cursor_left(Cursor* cursor);
void cursor_right(Cursor* cursor);

// Get the hex neighbors of the cursor.
void cursor_neighbors(const Cursor* cursor, HexNeighbors* neighbors);
