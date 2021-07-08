#include "statistics.h"

typedef struct {
    double render_ave_ms;
    double update_ave_ms;
    uint32_t total_frames;
} Statistics;

static Statistics _statistics;

void statistics_update(uint64_t update_time_ms, uint64_t render_time_ms) {
    const double smoothing = 0.9f;
    _statistics.update_ave_ms = (_statistics.update_ave_ms * smoothing) + ((double)update_time_ms * (1.0f - smoothing));
    _statistics.render_ave_ms = (_statistics.render_ave_ms * smoothing) + ((double)render_time_ms * (1.0f - smoothing));
    _statistics.total_frames++;
}

double statistics_fps(void) {
    return 1000.0f / _statistics.render_ave_ms;
}

uint32_t statistics_total_frames(void) {
    return _statistics.total_frames;
}
