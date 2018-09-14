#pragma once

#include <stdint.h> // uint32_t

inline
uint32_t
lxt_rand32(uint32_t * const seed)
{
    uint32_t x = *seed;
    
    x ^= x << 13;
    x ^= (x & 0xffffffffUL) >> 17;
    x ^= x << 5;
    
    *seed = x;
    
    return (x & 0xffffffffUL);
}
