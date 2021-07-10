#pragma once

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    Point top_left;
    Point bottom_right;
    int width;
    int height;
} Rectangle;
