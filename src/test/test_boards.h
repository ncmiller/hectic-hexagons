#pragma once

#include "hex.h"
#include "constants.h"

#define BOARD_SIZE (HEX_NUM_COLUMNS * HEX_NUM_ROWS)

extern HexType g_test_board_yellow_starflower[BOARD_SIZE];
extern HexType g_test_board_six_black_pearls[BOARD_SIZE];

void test_boards_print_current(void);
void test_boards_load(HexType board[BOARD_SIZE]);
