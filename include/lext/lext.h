#pragma once

#include <stddef.h> // size_t
#include <stdint.h> // uint32_t

struct lxt_opts {
    char const * generator;
    uint32_t * seed;
};

void lxt_gen(char * result,
             size_t length,
             char const * pattern,
             struct lxt_opts);
