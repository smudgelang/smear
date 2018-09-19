#ifndef NUMBER_H
#define NUMBER_H

#include <stdint.h>

typedef struct uint128_t {
    uint64_t lo;
    uint64_t hi;
} uint128_t;

int lt128(uint128_t, uint128_t);
int gt128(uint128_t, uint128_t);
int le128(uint128_t, uint128_t);
int ge128(uint128_t, uint128_t);
int eq128(uint128_t, uint128_t);
int ne128(uint128_t, uint128_t);

uint128_t add128(uint128_t, uint128_t);
uint128_t sub128(uint128_t, uint128_t);
uint128_t mul128(uint64_t, uint64_t);
uint128_t div128(uint128_t, uint64_t);

#endif
