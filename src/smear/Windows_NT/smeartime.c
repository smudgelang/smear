#include <Winbase.h>
#include "smeartime.h"

#define NANOSECONDS_PER_SECOND 1000000000

uint64_t get_now_ns(void)
{
    struct timespec ts;
    uint64_t now;

    now = GetTickCount64();
    return now * 1000; // Will overflow after 584,542 years of uptime.
}
