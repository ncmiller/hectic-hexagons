#pragma once

#include <stdint.h>

typedef struct {
    double render_ave_ns;
    double update_ave_ns;
    uint32_t total_frames;
} Statistics;

void statistics_update(uint64_t update_time_ns, uint64_t render_time_ns);
double statistics_fps(void);
Statistics* statistics_get(void);
