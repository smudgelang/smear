#define _POSIX_C_SOURCE 199309L
#include <time.h>
#include "smeartime.h"

#define NANOSECONDS_PER_SECOND 1000000000

uint64_t get_now_ns(void)
{
    struct timespec ts;
    uint64_t now;

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    now = ts.tv_nsec;
    now += ts.tv_sec * NANOSECONDS_PER_SECOND;
    return now;
}
