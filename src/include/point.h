#pragma once

typedef struct {
    float x;
    float y;
} Point;

typedef struct {
    Point top_left;
    Point bottom_right;
    int width;
    int height;
} Rectangle;
