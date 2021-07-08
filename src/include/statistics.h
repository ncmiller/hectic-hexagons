#pragma once

#include <stdint.h>

void statistics_update(uint64_t update_time_ms, uint64_t render_time_ms);
double statistics_fps(void);
uint32_t statistics_total_frames(void);
