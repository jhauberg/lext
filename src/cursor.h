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

enum lxt_cursor_direction {
    LXT_CURSOR_DIRECTION_FORWARD,
    LXT_CURSOR_DIRECTION_REVERSE
};

int32_t lxt_cursor_write(struct lxt_cursor *, struct lxt_token);

/**
 * Count the amount of whitespace to the left or right of a pointed string.
 *
 * Examples:
 *
 *      ↓
 *     [••abc•••]    (forward, spaces = 2)
 *       ↓
 *     [••abc•••]    (forward, spaces = 1)
 *        ↓
 *     [••abc•••]    (forward, spaces = 0)
 *          ↓
 *     [••abc•••]    (forward, spaces = 0)
 *           ↓
 *     [••abc•••]    (forward, spaces = 3)
 *        ↓
 *     [••abc•••]    (reverse, spaces = 0)
 *       ↓
 *     [••abc•••]    (reverse, spaces = 2)
 *            ↓
 *     [••abc•••]    (reverse, spaces = 2)
 *
 */
size_t lxt_cursor_spaces(char const * text,
                         enum lxt_cursor_direction);
