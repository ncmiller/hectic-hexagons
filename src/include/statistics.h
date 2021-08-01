#pragma once

#include <stdint.h>

typedef struct {
    double render_ave_ns;
    double update_ave_ns;
    double loop_iter_ave_ns;
} Statistics;

void statistics_update(uint64_t update_time_ns, uint64_t render_time_ns, uint64_t loop_iter_time_ns);
double statistics_fps(void);
Statistics* statistics_get(void);
