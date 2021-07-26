#include "constants.h"
#include "hex.h"
#include "window.h"

Constants g_constants = {0};

static Point hex_to_screen(int q, int r) {
    Point p;
    bool q_even = ((q & 1) == 0);
    p.x = g_constants.board.x + (int)round(q * 0.75f * g_constants.hex_w);
    if (q_even) {
        p.y = g_constants.board.y + (int)round(g_constants.hex_h * (r + 0.5f));
    } else {
        p.y = g_constants.board.y + (int)round(g_constants.hex_h * r);
    }
    return p;
}

bool constants_init(void) {
    const double s = HEX_WIDTH / 2.0f;
    const double h = sqrt(3) * s;
    const double w = 2.0f * s;
    g_constants.hex_s = s;
    g_constants.hex_h = h;
    g_constants.hex_w = w;

    g_constants.board_width = 5.0f * w + 4.0 * (w / 2.0f) + 0.75f * w;
    g_constants.board_height = h * HEX_NUM_ROWS;
    g_constants.board.x = LOGICAL_WINDOW_WIDTH / 2 - g_constants.board_width / 2;
    g_constants.board.y = LOGICAL_WINDOW_HEIGHT / 2 - g_constants.board_height / 2;

    // Pre-compute screen space point of upper-left corner of hex coord
    for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
        for (int r = 0; r < HEX_NUM_ROWS; r++) {
            g_constants.hex_to_screen[q][r] = hex_to_screen(q, r);
        }
        g_constants.hex_spawn_point[q] = hex_to_screen(q, -4);
    }

    return true;
}

Point transform_hex_to_screen(int q, int r) {
    return g_constants.hex_to_screen[q][r];
}
