#pragma once

#include <stddef.h> // size_t
#include <stdint.h> // int32_t

struct lxt_token;

/**
 * Represents a cursor in a writable buffer.
 *
 * The length specifies the maximum length/size of the pointed buffer.
 *
 * The offset determines the location from which to write to the buffer
 * (best conceptualized as "the cursor", or caret).
 *
 * For example:
 *
 *          |     (length = 5)
 *     [•••••]    (buffer)
 *      ^         (offset = 0)
 *
 */
struct lxt_cursor {
    char * buffer;
    size_t offset;
    size_t length;
};

int32_t lxt_cursor_write(struct lxt_cursor *, struct lxt_token);
