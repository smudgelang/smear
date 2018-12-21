#include <assert.h>
#include "number.h"

int main(void)
{
    uint128_t zero = {0, 0};
    uint128_t one  = {1, 0};
    uint128_t two  = {2, 0};
    uint128_t four = {4, 0};
    uint128_t twotothe15 = {1ull<<15, 0};
    uint128_t twotothe16 = {1ull<<16, 0};
    uint128_t twotothe17 = {1ull<<17, 0};
    uint128_t twotothe32minus1 = {(1ull<<32) - 1, 0};
    uint128_t twotothe32 = {1ull<<32, 0};
    uint128_t twotothe49 = {1ull<<49, 0};
    uint128_t twotothe63 = {1ull<<63, 0};
    uint128_t twotothe64minus2 = {-2, 0};
    uint128_t twotothe64minus1 = {-1, 0};
    uint128_t twotothe64 = {0, 1};
    uint128_t twotothe64plus1 = {1, 1};
    uint128_t twotothe65minus4 = {-4, 1};
    uint128_t twotothe65minus2 = {-2, 1};
    uint128_t twotothe65minus1 = {-1, 1};
    uint128_t twotothe65 = {0, 2};
    uint128_t twotothe65plus1 = {1, 2};
    uint128_t twotothe65plus2 = {2, 2};
    uint128_t twotothe66 = {0, 4};
    uint128_t twotothe66plus1 = {1, 4};
    uint128_t twotothe128minus2 = {-2, -1};
    uint128_t twotothe128minus1 = {-1, -1};

    // cast
    assert(cast128(1234567890).lo == 1234567890);
    assert(cast128(1234567890).hi == 0);

    // lt true
    assert(lt128(one, two));
    assert(lt128(twotothe64, twotothe65));
    assert(lt128(twotothe66, twotothe66plus1));

    // lt false
    assert(!lt128(four, two));
    assert(!lt128(two, two));
    assert(!lt128(twotothe66, twotothe65));
    assert(!lt128(twotothe65, twotothe65));
    assert(!lt128(twotothe66plus1, twotothe66));
    assert(!lt128(twotothe66plus1, twotothe66plus1));

    // gt true
    assert(gt128(four, two));
    assert(gt128(twotothe66, twotothe65));
    assert(gt128(twotothe66plus1, twotothe66));

    // gt false
    assert(!gt128(one, two));
    assert(!gt128(two, two));
    assert(!gt128(twotothe64, twotothe65));
    assert(!gt128(twotothe65, twotothe65));
    assert(!gt128(twotothe66, twotothe66plus1));
    assert(!gt128(twotothe66plus1, twotothe66plus1));

    // le true
    assert(le128(one, two));
    assert(le128(two, two));
    assert(le128(twotothe64, twotothe65));
    assert(le128(twotothe65, twotothe65));
    assert(le128(twotothe66, twotothe66plus1));
    assert(le128(twotothe66plus1, twotothe66plus1));

    // le false
    assert(!le128(four, two));
    assert(!le128(twotothe66, twotothe65));
    assert(!le128(twotothe66plus1, twotothe66));

    // ge true
    assert(ge128(four, two));
    assert(ge128(two, two));
    assert(ge128(twotothe66, twotothe65));
    assert(ge128(twotothe65, twotothe65));
    assert(ge128(twotothe66plus1, twotothe66));
    assert(ge128(twotothe66plus1, twotothe66plus1));

    // ge false
    assert(!ge128(one, two));
    assert(!ge128(twotothe64, twotothe65));
    assert(!ge128(twotothe66, twotothe66plus1));

    // eq true
    assert(eq128(two, two));
    assert(eq128(twotothe65, twotothe65));
    assert(eq128(twotothe66plus1, twotothe66plus1));

    // eq false
    assert(!eq128(four, two));
    assert(!eq128(twotothe66, twotothe65));
    assert(!eq128(twotothe66plus1, twotothe66));

    // ne true
    assert(ne128(four, two));
    assert(ne128(twotothe66, twotothe65));
    assert(ne128(twotothe66plus1, twotothe66));

    // ne false
    assert(!ne128(two, two));
    assert(!ne128(twotothe65, twotothe65));
    assert(!ne128(twotothe66plus1, twotothe66plus1));

    // add small
    assert(eq128(two,               add128(zero, two)));
    assert(eq128(two,               add128(two, zero)));
    assert(eq128(four,              add128(two, two)));

    // add mixed
    assert(eq128(twotothe64,        add128(zero, twotothe64)));
    assert(eq128(twotothe64,        add128(twotothe64, zero)));
    assert(eq128(twotothe65minus2,  add128(twotothe64minus1, twotothe64minus1)));
    assert(eq128(twotothe65,        add128(twotothe64minus1, twotothe64plus1)));
    assert(eq128(twotothe65,        add128(twotothe64plus1, twotothe64minus1)));
    assert(eq128(twotothe66plus1,   add128(one, twotothe66)));
    assert(eq128(twotothe66plus1,   add128(twotothe66, one)));

    // add large
    assert(eq128(twotothe65,        add128(twotothe64, twotothe64)));
    assert(eq128(twotothe65plus2,   add128(twotothe64plus1, twotothe64plus1)));
    assert(eq128(twotothe66,        add128(twotothe65minus2, twotothe65plus2)));
    assert(eq128(twotothe66,        add128(twotothe65plus2, twotothe65minus2)));

    // add overflow
    assert(eq128(zero,              add128(twotothe128minus1, one)));
    assert(eq128(one,               add128(twotothe128minus1, two)));
    assert(eq128(twotothe128minus2, add128(twotothe128minus1, twotothe128minus1)));

    // sub small
    assert(eq128(two,               sub128(two, zero)));
    assert(eq128(zero,              sub128(two, two)));
    assert(eq128(two,               sub128(four, two)));

    // sub mixed
    assert(eq128(twotothe64,        sub128(twotothe64, zero)));
    assert(eq128(zero,              sub128(twotothe64, twotothe64)));
    assert(eq128(twotothe64minus1,  sub128(twotothe65minus2, twotothe64minus1)));
    assert(eq128(twotothe64plus1,   sub128(twotothe65, twotothe64minus1)));
    assert(eq128(twotothe64minus1,  sub128(twotothe65, twotothe64plus1)));
    assert(eq128(twotothe66,        sub128(twotothe66plus1, one)));
    assert(eq128(one,               sub128(twotothe66plus1, twotothe66)));

    // sub large
    assert(eq128(twotothe64,        sub128(twotothe65, twotothe64)));
    assert(eq128(twotothe64plus1,   sub128(twotothe65plus2, twotothe64plus1)));
    assert(eq128(twotothe65minus2,  sub128(twotothe66, twotothe65plus2)));
    assert(eq128(twotothe65plus2,   sub128(twotothe66, twotothe65minus2)));

    // sub underflow
    assert(eq128(one,               sub128(zero, twotothe128minus1)));
    assert(eq128(two,               sub128(one, twotothe128minus1)));
    assert(eq128(twotothe128minus1, sub128(twotothe128minus2, twotothe128minus1)));

    // mul small
    assert(eq128(zero,              mul128(0, 2)));
    assert(eq128(zero,              mul128(2, 0)));
    assert(eq128(two,               mul128(1, 2)));
    assert(eq128(two,               mul128(2, 1)));
    assert(eq128(four,              mul128(2, 2)));

    // mul large
    assert(eq128(twotothe64minus2,  mul128((1ull<<63)-1, 2)));
    assert(eq128(twotothe64,        mul128(1ull<<63, 2)));
    assert(eq128(twotothe64,        mul128(1ull<<32, 1ull<<32)));
    assert(eq128(twotothe65minus4,  mul128((1ull<<63)-1, 4)));
    assert(eq128(twotothe65,        mul128(2ull<<32, 1ull<<32)));

    // div digits in numerator < digits in denominator
    assert(eq128(zero,              div128(zero, 1ull<<16)));
    assert(eq128(zero,              div128(one, 1ull<<16)));
    assert(eq128(zero,              div128(two, 1ull<<16)));
    assert(eq128(zero,              div128(twotothe16, 1ull<<32)));
    assert(eq128(zero,              div128(twotothe32, 1ull<<48)));

    // div numerator small, denominator < 2**16
    assert(eq128(zero,              div128(zero, 1)));
    assert(eq128(one,               div128(one, 1)));
    assert(eq128(one,               div128(two, 2)));
    assert(eq128(two,               div128(two, 1)));
    assert(eq128(two,               div128(four, 2)));
    assert(eq128(two,               div128(twotothe16, 1ull<<15)));
    assert(eq128(twotothe15,        div128(twotothe16, 2)));
    assert(eq128(twotothe17,        div128(twotothe32, 1ull<<15)));

    // div numerator large, denominator < 2**16
    assert(eq128(twotothe64,        div128(twotothe64, 1)));
    assert(eq128(twotothe64plus1,   div128(twotothe64plus1, 1)));
    assert(eq128(twotothe63,        div128(twotothe64, 2)));
    assert(eq128(twotothe63,        div128(twotothe64plus1, 2)));
    assert(eq128(twotothe49,        div128(twotothe64, 1ull<<15)));
    assert(eq128(twotothe64plus1,   div128(twotothe65plus2, 2)));

    // div numerator small
    assert(eq128(one,               div128(twotothe16, 1ull<<16)));
    assert(eq128(twotothe16,        div128(twotothe32, 1ull<<16)));
    assert(eq128(twotothe17,        div128(twotothe49, 1ull<<32)));
    assert(eq128(twotothe32minus1,  div128(twotothe64minus1, 1ull<<32)));

    // div numerator large
    assert(eq128(twotothe49,        div128(twotothe65, 1ull<<16)));
    assert(eq128(twotothe49,        div128(twotothe65plus1, 1ull<<16)));
    assert(eq128(twotothe65minus1,  div128(twotothe128minus1, 1ull<<63)));
    assert(eq128(twotothe65minus1,  div128(twotothe128minus2, 1ull<<63)));

    return 0;
}

