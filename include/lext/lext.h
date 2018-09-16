#pragma once

#include <stddef.h> // size_t
#include <stdint.h> // uint32_t

#define LXT_VERSION_MAJOR (0)
#define LXT_VERSION_MINOR (2)
#define LXT_VERSION_PATCH (0)

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

extern struct lxt_opts const LXT_OPTS_NONE;

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
