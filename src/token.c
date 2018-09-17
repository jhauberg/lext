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

static void lxt_token_trim_leading(struct lxt_token *);
static void lxt_token_trim_trailing(struct lxt_token *);

static
void
lxt_token_trim_leading(struct lxt_token * const token)
{
    if (token->length == 0) {
        return;
    }
    
    size_t const leading = lxt_count_spaces(token->start,
                                            LXT_DIRECTION_FORWARD);
    
    token->start = token->start + leading;
    
    if (token->length >= leading) {
        token->length -= leading;
    } else {
        token->length = 0;
    }
}

static
void
lxt_token_trim_trailing(struct lxt_token * const token)
{
    if (token->length == 0) {
        return;
    }
    
    size_t const trailing = lxt_count_spaces(token->start + token->length - 1,
                                             LXT_DIRECTION_REVERSE);
    
    if (token->length >= trailing) {
        token->length -= trailing;
    } else {
        token->length = 0;
    }
}

void
lxt_token_trim(struct lxt_token * const token)
{
    lxt_token_trim_leading(token);
    lxt_token_trim_trailing(token);
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
lxt_token_validates(struct lxt_token token,
                    enum lxt_kind const kind)
{
    if (kind != LXT_KIND_VARIABLE &&
        kind != LXT_KIND_CONTAINER &&
        kind != LXT_KIND_GENERATOR) {
        return true;
    }
    
    for (size_t i = 0; i < token.length; i++) {
        char const character = *(token.start + i);
        
        if (!lxt_token_character(character)) {
            return false;
        }
    }
    
    return true;
}

bool
lxt_token_character(char const character)
{
    bool const is_numerical =
        (character >= '0' &&
         character <= '9');
    
    bool const is_alphabetical =
        (character >= 'A' && character <= 'Z') ||
        (character >= 'a' && character <= 'z');
    
    return is_numerical || is_alphabetical || character == '_';
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
