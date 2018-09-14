#pragma once

#include <stddef.h> // size_t
#include <stdint.h> // uint32_t

/**
 * Represents optional settings that affect a generated result.
 */
struct lxt_opts {
    /**
     * Specifies the name of a specific generator to run.
     */
    char const * generator;
    /**
     * Specifies the randomization seed.
     */
    uint32_t * seed;
};

/**
 * Represents a result code, indicating success or not.
 */
enum lxt_error {
    LXT_ERROR_NONE,
    LXT_ERROR_INVALID_TEMPLATE,
    LXT_ERROR_GENERATOR_NOT_FOUND
};

/**
 * Generate a random result into buffer given a template pattern.
 *
 * The result is truncated if it exceeds the specified length.
 */
enum lxt_error lxt_gen(char * buffer,
                       size_t length,
                       char const * pattern,
                       struct lxt_opts);
