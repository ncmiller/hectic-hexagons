#include "constants.h"
#include "hex.h"
#include "window.h"

Constants g_constants = {0};

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
    return true;
}
