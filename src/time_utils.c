#include "time_utils.h"
#include <time.h>
#include <math.h>

#define NS_PER_SECOND (1000000000)

uint64_t now_ms(void) {
    return now_us() / 1000;
}

uint64_t now_us(void) {
    return now_ns() / 1000;
}

uint64_t now_ns(void) {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (ts.tv_sec * NS_PER_SECOND + ts.tv_nsec);
}

int32_t ms_to_frames(int32_t ms) {
    return (int32_t)ceil((double)ms / (double)MS_PER_FRAME);
}
