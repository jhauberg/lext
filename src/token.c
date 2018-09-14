#include "token.h" // lxt_token, lxt_token_*

#include <stddef.h> // size_t
#include <stdint.h> // int32_t
#include <stdbool.h> // bool
#include <string.h> // strncmp

#include <ctype.h> // isspace

enum lxt_direction {
    LXT_DIRECTION_FORWARD,
    LXT_DIRECTION_REVERSE
};

/**
 * Count the amount of whitespace to the left or right.
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
static size_t lxt_count_spaces(char const * text,
                               enum lxt_direction);

void
lxt_token_trim(struct lxt_token * const token)
{
    size_t const leading = lxt_count_spaces(token->start,
                                            LXT_DIRECTION_FORWARD);
    
    token->start = token->start + leading;
    token->length -= leading;
    
    size_t const trailing = lxt_count_spaces(token->start + token->length - 1,
                                             LXT_DIRECTION_REVERSE);
    
    token->length -= trailing;
}

bool
lxt_token_equals(struct lxt_token const * const token,
                 char const * const name,
                 size_t const length)
{
    if (token->length != length) {
        return false;
    }
    
    if (strncmp(token->start, name, token->length) != 0) {
        return false;
    }
    
    return true;
}

bool
lxt_token_delimiter(char const character,
                    enum lxt_kind * const kind)
{
    *kind = LXT_KIND_NONE;
    
    switch (character) {
        case '(': {
            *kind = LXT_KIND_CONTAINER;
        } break;
            
        case ',':
            /* fall through */
        case ')': {
            *kind = LXT_KIND_CONTAINER_ENTRY;
        } break;
            
        case '<': {
            *kind = LXT_KIND_GENERATOR;
        } break;
            
        case '>': {
            *kind = LXT_KIND_SEQUENCE;
        } break;
            
        default:
            break;
    }
    
    if (*kind == LXT_KIND_NONE) {
        return false;
    }
    
    return true;
}

static
size_t
lxt_count_spaces(char const * text,
                 enum lxt_direction const direction)
{
    size_t length = 0;
    int32_t offset = direction == LXT_DIRECTION_REVERSE ? -1 : 1;
    
    while (*text && isspace(*text)) {
        length += 1;
        
        text += offset;
    }
    
    return length;
}
