#include "test_boards.h"
#include "game_state.h"
#include <stdio.h>

#define GR HEX_TYPE_GREEN
#define BL HEX_TYPE_BLUE
#define YL HEX_TYPE_YELLOW
#define MG HEX_TYPE_MAGENTA
#define PL HEX_TYPE_PURPLE
#define RD HEX_TYPE_RED
#define SF HEX_TYPE_STARFLOWER
#define BU HEX_TYPE_BLACK_PEARL_UP
#define BD HEX_TYPE_BLACK_PEARL_DOWN
#define XX HEX_TYPE_INVALID

HexType g_test_board_yellow_starflower[BOARD_SIZE] = {
    RD, YL, PL, GR, RD, RD, YL, BL, PL, BL,
    RD, PL, BL, YL, BL, BL, PL, BL, PL, GR,
    BL, GR, PL, GR, GR, RD, YL, YL, PL, GR,
    BL, RD, GR, RD, RD, BL, GR, RD, YL, GR,
    RD, GR, YL, YL, YL, BL, YL, GR, RD, RD,
    BL, PL, RD, BL, YL, BL, RD, BL, YL, PL,
    BL, YL, BL, YL, GR, PL, YL, YL, GR, RD,
    PL, RD, BL, PL, BL, BL, PL, BL, BL, PL,
    XX, PL, XX, YL, XX, RD, XX, GR, XX, BL,
};

// https://www.youtube.com/watch?v=WIoabsFQv1s
HexType g_test_board_six_black_pearls[BOARD_SIZE] = {
    PL, YL, YL, RD, BL, RD, PL, YL, PL, GR,
    PL, GR, SF, SF, SF, SF, SF, BL, RD, GR,
    GR, BL, SF, BL, SF, PL, SF, YL, GR, PL,
    PL, YL, RD, RD, RD, RD, RD, SF, SF, YL,
    PL, BL, RD, BL, YL, PL, RD, PL, SF, RD,
    RD, SF, YL, RD, RD, RD, YL, SF, RD, GR,
    YL, SF, SF, SF, SF, SF, SF, SF, GR, GR,
    BL, RD, BL, RD, GR, GR, GR, GR, RD, BL,
    XX, YL, XX, RD, XX, RD, XX, PL, XX, PL,
};

static const char* type_to_str[NUM_HEX_TYPES] = {
    [HEX_TYPE_GREEN] = "GR",
    [HEX_TYPE_BLUE] = "BL",
    [HEX_TYPE_YELLOW] = "YL",
    [HEX_TYPE_MAGENTA] = "MG",
    [HEX_TYPE_PURPLE] = "PL",
    [HEX_TYPE_RED] = "RD",
    [HEX_TYPE_STARFLOWER] = "SF",
    [HEX_TYPE_BLACK_PEARL_UP] = "BU",
    [HEX_TYPE_BLACK_PEARL_DOWN] = "BD",
};

void test_boards_print_current(void) {
    printf("\n\n");
    for (int r = 0; r < HEX_NUM_ROWS; r++) {
        for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
            const Hex* hex = &g_state.hexes[q][r];
            if (hex->is_valid && hex->type < NUM_HEX_TYPES) {
                printf("%s, ", type_to_str[hex->type]);
            } else {
                printf("XX, ");
            }
        }
        printf("\n");
    }
    printf("\n");
}

void test_boards_load(HexType board[BOARD_SIZE]) {
    for (int r = 0; r < HEX_NUM_ROWS; r++) {
        for (int q = 0; q < HEX_NUM_COLUMNS; q++) {
            Hex* hex = &g_state.hexes[q][r];
            if (hex->is_valid) {
                hex->type = board[r * HEX_NUM_COLUMNS + q];
            }
        }
    }
}
