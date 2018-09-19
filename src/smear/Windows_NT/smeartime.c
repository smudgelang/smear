#include <windows.h>
#include <Winbase.h>
#include <assert.h>
#include "smeartime.h"
#include "number.h"

#define NANOSECONDS_PER_SECOND 1000000000

uint64_t get_now_ns(void)
{
    // Per MSDN, "[t]he frequency [...] is fixed at system boot and is
    // consistent across all processors[, so it] need only be queried
    // upon [...] initialization."  The units are counts/second, and it
    // is guaranteed non-zero on WinXP or later.
    static LARGE_INTEGER freq = { .QuadPart = 0 };
    if (freq.QuadPart == 0) {
        QueryPerformanceFrequency(&freq);
    }

    // Monotonic, <1us time stamp, units are counts
    // Guaranteed to succeed on WinXP or later.
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    // For clock time, use GetSystemTimePreciseAsFileTime
    uint128_t precise, ns;
    precise = mul128(now.QuadPart, NANOSECONDS_PER_SECOND);
    ns = div128(precise, freq.QuadPart);
    assert(ns.hi == 0);
    return ns.lo;
}
