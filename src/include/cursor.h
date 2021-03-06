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
} Cursor;

void cursor_init(Cursor* cursor);

// Return true if cursor was moved
bool cursor_up(Cursor* cursor);
bool cursor_down(Cursor* cursor);
bool cursor_left(Cursor* cursor);
bool cursor_right(Cursor* cursor);

// Get the hex neighbors of the cursor.
void cursor_neighbors(const Cursor* cursor, HexNeighbors* neighbors);

// Returns true if query_hex_coord is under the cursor
bool cursor_contains_hex(const Cursor* cursor, HexCoord query_hex_coord);

void cursor_print(void);
