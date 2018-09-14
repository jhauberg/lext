#pragma once

#include <stddef.h> // size_t
#include <stdbool.h> // bool

/**
 * Represents a tokenized string in a template.
 *
 * A token is simply a pointer/length to a location inside a full template.
 *
 * To determine where a token ends, consumers must use its specified length,
 * as the pointed string is *not* null-terminated.
 */
struct lxt_token {
    char const * start;
    size_t length;
};

/**
 * Represents the kind or type of a token.
 */
enum lxt_kind {
    /**
     * Token is of no particular kind and can be ignored.
     */
    LXT_KIND_NONE,
    LXT_KIND_CONTAINER,
    LXT_KIND_CONTAINER_ENTRY,
    LXT_KIND_GENERATOR,
    LXT_KIND_SEQUENCE,
    LXT_KIND_VARIABLE,
    LXT_KIND_TEXT,
    LXT_KIND_COMMENT
};

/**
 * Trim leading and trailing whitespace from a token.
 *
 * Since a token is just a pointer and length into a full template,
 * trimming is done by counting the number of whitespaces from both ends and
 * simply offsetting said pointer and length until it both starts- and
 * ends in non-whitespace.
 *
 * Example (each • representing whitespace):
 *
 *      ↓       |     (length = 9)
 *     [••text•••]
 *
 *   Results in:
 *
 *        ↓  |        (length = 4)
 *     [••text•••]
 *
 */
void lxt_token_trim(struct lxt_token *);
bool lxt_token_equals(struct lxt_token const *,
                      char const * name,
                      size_t length);
/**
 * Determine whether a character matches a token delimiter.
 */
bool lxt_token_delimiter(char character, enum lxt_kind *);
