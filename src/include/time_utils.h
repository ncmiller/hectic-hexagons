#pragma once

#include <stdint.h>

#define MS_PER_FRAME 16.667f

// Current time, in milliseconds
uint64_t now_ms(void);

// Current time, in microseconds
uint64_t now_us(void);

// Current time, in nanoseconds
uint64_t now_ns(void);

int32_t ms_to_frames(int32_t ms);
