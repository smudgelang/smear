#include <windows.h>
#include <Winbase.h>
#include "smeartime.h"

#define NANOSECONDS_PER_MILLISECOND 1000

uint64_t get_now_ns(void)
{
    uint64_t now;

    now = GetTickCount64();
    return now * NANOSECONDS_PER_MILLISECOND;
}
