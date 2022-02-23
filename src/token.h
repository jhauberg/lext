#pragma once

#include <stddef.h> // size_t
#include <stdbool.h> // bool

#define CONTAINER_START '('
#define CONTAINER_END ')'
#define CONTAINER_DELIMITER ','
#define VARIABLE '@'
#define COMMENT '#'
#define GENERATOR_START '<'
#define GENERATOR_END '>'

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

/**
 * Determine whether a token equals another token.
 */
bool lxt_token_equals(struct lxt_token,
                      struct lxt_token other);

/**
 * Determine whether a token identifier consist of only valid characters.
 */
bool lxt_token_validates(struct lxt_token, enum lxt_kind);
/**
 * Determine whether a character is a valid token character.
 */
bool lxt_token_character(char character);

/**
 * Determine whether a token starts at a given pattern.
 */
bool lxt_token_starts(enum lxt_kind *, char const * pattern);
/**
 * Determine whether a token ends at a given pattern.
 */
bool lxt_token_ends(enum lxt_kind *, char const * pattern);
