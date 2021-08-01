#include "statistics.h"

static Statistics _statistics;

void statistics_update(uint64_t update_time_ns, uint64_t render_time_ns, uint64_t loop_iter_time_ns) {
    const double smoothing = 0.9f;
    _statistics.update_ave_ns = (_statistics.update_ave_ns * smoothing) + ((double)update_time_ns * (1.0f - smoothing));
    _statistics.render_ave_ns = (_statistics.render_ave_ns * smoothing) + ((double)render_time_ns * (1.0f - smoothing));
    _statistics.loop_iter_ave_ns = (_statistics.loop_iter_ave_ns * smoothing) + ((double)loop_iter_time_ns * (1.0f - smoothing));
}

double statistics_fps(void) {
    return 1000000000.0f / _statistics.loop_iter_ave_ns;
}

Statistics* statistics_get(void) {
    return &_statistics;
}
