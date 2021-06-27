#pragma once

typedef struct {
    bool follow_mouse;
    Position hex_position;
} Game;

bool game_init(void);
void game_update(void);
