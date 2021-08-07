#include "cursor.h"
#include "hex.h"
#include "game_state.h"
#include <macros.h>

static Hex* anchor(const Cursor* cursor) {
    return hex_at(cursor->hex_anchor.q, cursor->hex_anchor.r);
}

static void update_screen_point(Cursor* cursor) {
    Point p = transform_hex_to_screen(cursor->hex_anchor.q, cursor->hex_anchor.r);

    if (cursor->position == CURSOR_POS_RIGHT) {
        cursor->screen_point.x = p.x + HEX_WIDTH;
        cursor->screen_point.y = p.y + HEX_HEIGHT / 2;
    } else if (cursor->position == CURSOR_POS_LEFT) {
        cursor->screen_point.x = p.x;
        cursor->screen_point.y = p.y + HEX_HEIGHT / 2;
    } else if (cursor->position == CURSOR_POS_ON) {
        cursor->screen_point.x = p.x + HEX_WIDTH / 2;
        cursor->screen_point.y = p.y + HEX_HEIGHT / 2;
    }
}

void cursor_init(Cursor* cursor) {
    cursor->hex_anchor = (HexCoord){ .q = HEX_NUM_COLUMNS / 2, .r = HEX_NUM_ROWS / 2 };
    cursor->position = CURSOR_POS_LEFT;
    update_screen_point(cursor);
}

bool cursor_up(Cursor* cursor) {
    int q = cursor->hex_anchor.q;
    int r = cursor->hex_anchor.r;

    if (cursor->position == CURSOR_POS_RIGHT) {
        if (r == 0) {
            return false;
        }
        cursor->hex_anchor = hex_neighbor_coord(q, r, HEX_NEIGHBOR_TOP_RIGHT);
        cursor->position = CURSOR_POS_LEFT;
    } else if (cursor->position == CURSOR_POS_LEFT) {
        if (r == 0) {
            return false;
        }
        cursor->hex_anchor = hex_neighbor_coord(q, r, HEX_NEIGHBOR_TOP_LEFT);
        cursor->position = CURSOR_POS_RIGHT;
    } else if (cursor->position == CURSOR_POS_ON) {
        cursor->hex_anchor = hex_neighbor_coord(q, r, HEX_NEIGHBOR_TOP_LEFT);
        cursor->position = CURSOR_POS_RIGHT;
    }

    update_screen_point(cursor);
    return true;
}

bool cursor_down(Cursor* cursor) {
    int q = cursor->hex_anchor.q;
    int r = cursor->hex_anchor.r;

    if (cursor->position == CURSOR_POS_RIGHT) {
        if (r == HEX_NUM_ROWS - 2) {
            return false;
        }
        cursor->hex_anchor = hex_neighbor_coord(q, r, HEX_NEIGHBOR_BOTTOM_RIGHT);
        cursor->position = CURSOR_POS_LEFT;
    } else if (cursor->position == CURSOR_POS_LEFT) {
        if (r == HEX_NUM_ROWS - 1) {
            return false;
        }
        cursor->hex_anchor = hex_neighbor_coord(q, r, HEX_NEIGHBOR_BOTTOM_LEFT);
        cursor->position = CURSOR_POS_RIGHT;
    } else if (cursor->position == CURSOR_POS_ON) {
        cursor->hex_anchor = hex_neighbor_coord(q, r, HEX_NEIGHBOR_BOTTOM_LEFT);
        cursor->position = CURSOR_POS_RIGHT;
    }

    update_screen_point(cursor);
    return true;
}

bool cursor_left(Cursor* cursor) {
    int q = cursor->hex_anchor.q;

    if (cursor->position == CURSOR_POS_RIGHT) {
        if (q == 0) {
            return false;
        }
        HexType type = anchor(cursor)->type;
        if ((type == HEX_TYPE_STARFLOWER) ||
            (type == HEX_TYPE_BLACK_PEARL_UP) ||
            (type == HEX_TYPE_BLACK_PEARL_DOWN)) {
            cursor->position = CURSOR_POS_ON;
        } else {
            cursor->position = CURSOR_POS_LEFT;
        }
    } else if (cursor->position == CURSOR_POS_LEFT) {
        if (q <= 1) {
            return false;
        }
        cursor->hex_anchor.q -= 2;
        cursor->position = CURSOR_POS_RIGHT;
    } else if (cursor->position == CURSOR_POS_ON) {
        cursor->position = CURSOR_POS_LEFT;
    }

    update_screen_point(cursor);
    return true;
}

bool cursor_right(Cursor* cursor) {
    int q = cursor->hex_anchor.q;

    if (cursor->position == CURSOR_POS_RIGHT) {
        if (q >= HEX_NUM_COLUMNS - 2) {
            return false;
        }
        cursor->hex_anchor.q += 2;
        cursor->position = CURSOR_POS_LEFT;
    } else if (cursor->position == CURSOR_POS_LEFT) {
        if (q >= HEX_NUM_COLUMNS - 1) {
            return false;
        }
        HexType type = anchor(cursor)->type;
        if ((type == HEX_TYPE_STARFLOWER) ||
            (type == HEX_TYPE_BLACK_PEARL_UP) ||
            (type == HEX_TYPE_BLACK_PEARL_DOWN)) {
            cursor->position = CURSOR_POS_ON;
        } else {
            cursor->position = CURSOR_POS_RIGHT;
        }
    } else if (cursor->position == CURSOR_POS_ON) {
        cursor->position = CURSOR_POS_RIGHT;
    }

    update_screen_point(cursor);
    return true;
}

void cursor_neighbors(const Cursor* cursor, HexNeighbors* neighbors) {
    const int q = cursor->hex_anchor.q;
    const int r = cursor->hex_anchor.r;

    if (cursor->position == CURSOR_POS_ON) {
        const HexType type = anchor(cursor)->type;
        if (type == HEX_TYPE_STARFLOWER) {
            hex_neighbors(q, r, neighbors, ALL_NEIGHBORS);
        } else if (type == HEX_TYPE_BLACK_PEARL_UP) {
            hex_neighbors(q, r, neighbors, BLACK_PEARL_UP_NEIGHBORS);
        } else if (type == HEX_TYPE_BLACK_PEARL_DOWN) {
            hex_neighbors(q, r, neighbors, BLACK_PEARL_DOWN_NEIGHBORS);
        }
    } else if (cursor->position == CURSOR_POS_LEFT) {
        hex_neighbors(q, r, neighbors, TRIO_LEFT_NEIGHBORS);
    } else if (cursor->position == CURSOR_POS_RIGHT) {
        hex_neighbors(q, r, neighbors, TRIO_RIGHT_NEIGHBORS);
    } else {
        ASSERT(false && "Unexpected cursor position");
    }
}

void cursor_print(void) {
    const char* pos_str = "";
    if (g_state.cursor.position == CURSOR_POS_LEFT) {
        pos_str = "Left of";
    } else if (g_state.cursor.position == CURSOR_POS_RIGHT) {
        pos_str = "Right of";
    } else if (g_state.cursor.position == CURSOR_POS_ON) {
        pos_str = "On";
    } else {
        pos_str = "Unknown";
    }

    SDL_Log("Cursor %s (%d,%d)",
            pos_str,
            g_state.cursor.hex_anchor.q,
            g_state.cursor.hex_anchor.r);
}

bool cursor_contains_hex(const Cursor* cursor, HexCoord query_hex_coord) {
    const Hex* query_hex = hex_at(query_hex_coord.q, query_hex_coord.r);
    if (query_hex == anchor(cursor)) {
        return true;
    }

    HexNeighbors cursor_hexes = {0};
    cursor_neighbors(cursor, &cursor_hexes);

    for (int i = 0; i < cursor_hexes.num_neighbors; i++) {
        const Hex* n = hex_at(cursor_hexes.coords[i].q, cursor_hexes.coords[i].r);
        if (n == query_hex) {
            return true;
        }
    }

    return false;
}
