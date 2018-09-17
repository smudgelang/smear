#include <stdint.h>
#include "number.h"

int lt128(uint128_t n, uint128_t m)
{
    return n.hi < m.hi ||
           (n.hi == m.hi && n.lo < m.lo);
}

int gt128(uint128_t n, uint128_t m)
{
    return n.hi > m.hi ||
           (n.hi == m.hi && n.lo > m.lo);
}

int le128(uint128_t n, uint128_t m)
{
    return n.hi < m.hi ||
           (n.hi == m.hi && n.lo <= m.lo);
}

int ge128(uint128_t n, uint128_t m)
{
    return n.hi > m.hi ||
           (n.hi == m.hi && n.lo >= m.lo);
}

int eq128(uint128_t n, uint128_t m)
{
    return n.hi == m.hi && n.lo == m.lo;
}

int ne128(uint128_t n, uint128_t m)
{
    return n.hi != m.hi || n.lo != m.lo;
}

uint128_t add128(uint128_t n, uint128_t m)
{
    uint128_t sum = {0, 0};
    sum.lo = n.lo + m.lo;
    sum.hi = n.hi + m.hi +
             (sum.lo < n.lo ? 1 : 0);
    return sum;
}

uint128_t sub128(uint128_t n, uint128_t m)
{
    uint128_t diff = {0, 0};
    diff.lo = n.lo - m.lo;
    diff.hi = n.hi - m.hi -
              (diff.lo > n.lo ? 1 : 0);
    return diff;
}

uint128_t mul128(uint64_t n, uint64_t m)
{
    static const uint64_t mask = ((1ull << 32) - 1);
    uint128_t prod = {0, 0};
    uint64_t accum  = 0;
    uint64_t n_hi   = 0, n_lo   = 0;
    uint64_t m_hi   = 0, m_lo   = 0;
    uint64_t mid = 0;
    n_hi = n >> 32;
    n_lo = n & mask;
    m_hi = m >> 32;
    m_lo = m & mask;

    accum   = n_lo * m_lo;
    prod.lo = accum & mask;
    accum >>= 32;
    accum += n_lo * m_hi;
    mid = accum & mask;
    prod.hi = accum >> 32;

    accum  = n_hi * m_lo;
    mid += accum & mask;
    prod.lo += (mid & mask) << 32;
    prod.hi += mid >> 32;
    prod.hi += accum >> 32;
    prod.hi += n_hi * m_hi;
    return prod;
}

// This division algorithm was cribbed from Per Brinch
// Hanson's paper "Multiple-Length Division Revisited."
// It is like grade-school long division with a trick.
//
// To summarize, a prefix of each divisor and dividend
// is used to perform "trial iteration" of division,
// yielding an estimate of the quotient digit that is
// at most one-greater than the actual quotient digit.
// This is then corrected.
//
// The key is that three digits fit in a machine word.
// Then the division of two digits into three takes a
// single step.
//
// Here, a "digit" is 16 bits.  This was to make the
// calculation obvious.  It is possible to handle the
// 2-digit divisor case simply.  It is also possible to
// shave off a division in the >2-digit divisor case,
// but I don't want to bother with either of these
// optimizations for now.  The implementation is less
// obvious, and the base gets weird: floor(64/3) = 21.
//
// Of note is that normalization by a scaling factor is
// skipped, resulting in more corrections.

static const uint64_t qmask = ((1ull << 16) - 1);

static inline uint16_t dig16(uint128_t n, size_t d)
{
    if (d < 4) {
        return (n.lo >> (d * 16)) & qmask;
    } else if (d < 8) {
        return (n.hi >> ((d - 4) * 16)) & qmask;
    }
    return 0;
}

static inline void setdig16(uint128_t *n, size_t d, uint16_t m)
{
    if (d < 4) {
        uint64_t mask = ~(qmask << (d * 16));
        n->lo = (n->lo & mask) + (((uint64_t)m) << (d * 16));
    } else if (d < 8) {
        uint64_t mask = ~(qmask << ((d - 4) * 16));
        n->hi = (n->hi & mask) + (((uint64_t)m) << ((d - 4) * 16));
    }
}

static inline uint64_t min(uint64_t a, uint64_t b)
{
    return a < b ? a : b;
}

static uint128_t div128_16(uint128_t r, uint16_t d)
{
    // Algorithm 3
    size_t n = 8;
    // find number of significant digits in remainder
    while (dig16(r, n - 1) == 0 && n > 1 && --n);
    uint128_t q = {0, 0};
    uint32_t c = 0;
    for (size_t k = n; k-- > 0;) {
        c = (c << 16) + dig16(r, k);
        setdig16(&q, k, c / d);
        c %= d;
    }
    return q;
}

uint128_t div128(uint128_t r, uint64_t d64)
{
    size_t n = 8;
    size_t m = 4;

    // up cast since it is simpler to divide like types.
    uint128_t d = {0, 0};
    d.lo = d64;

    //r has n<=8 digits:
    //r = r_{n - 1} * 16^{n - 1} + ... + r_0
    while (dig16(r, n - 1) == 0 && n > 1 && --n);
    //d has m<=4 digits:
    //d = d_{m - 1} * 16^{m - 1} + ... + d_0
    while (dig16(d, m - 1) == 0 && m > 1 && --m);

    if (m > n) {
        return (uint128_t){0, 0};
    } else if (m == 1) {
        return div128_16(r, d64 & qmask);
    } else { // 2 <= m <= n
        // Algorithms 5 and 8, excluding normalization
        //q has n - m + 1 digits:
        //q = q_{n - m} * 16^{n - m} + ... + q_0
        uint128_t q = {0, 0};
        uint64_t d_2 = 0;

        d_2 = (((uint64_t)dig16(d, m - 1)) << 16)
            +             dig16(d, m - 2);

        for(int k = n - m; k >= 0; k--) {
            uint16_t q_e = 0;
            uint64_t r_3 = 0;

            r_3 = (((uint64_t)dig16(r, k + m)) << 32)
                + (((uint64_t)dig16(r, k + m - 1)) << 16)
                +             dig16(r, k + m - 2);
            q_e = min(r_3 / d_2, qmask);
            // 0 <= q_k <= q_e <= q_k + 1 <= 2^16 (per eqn. 17)

            // full lt is best-case constant-factor suboptimal
            uint128_t prod = mul128(q_e, d64);
            uint128_t r_m1 = {0, 0};
            for(size_t j = 0; j < m + 1; ++j) {
                setdig16(&r_m1, j, dig16(r, j + k));
            }
            if (lt128(r_m1, prod)) {
                q_e = q_e - 1;
                prod = sub128(prod, d);
            }
            setdig16(&q, k, q_e);
            // shift left k digits
            for(int j = m + k; j >= 0; --j) {
                setdig16(&prod, j, dig16(prod, j - k));
            }
            r = sub128(r, prod);
        }
        return q;
    }
}
