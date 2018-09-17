#include "token.h" // lxt_token, lxt_token_*
#include "cursor.h" // lxt_cursor_spaces

#include <stddef.h> // size_t
#include <stdbool.h> // bool
#include <string.h> // strncmp

static void lxt_token_trim_leading(struct lxt_token *);
static void lxt_token_trim_trailing(struct lxt_token *);

static
void
lxt_token_trim_leading(struct lxt_token * const token)
{
    if (token->length == 0) {
        return;
    }
    
    size_t const leading = lxt_cursor_spaces(token->start,
                                             LXT_CURSOR_DIRECTION_FORWARD);
    
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
    
    size_t const trailing = lxt_cursor_spaces(token->start + token->length - 1,
                                              LXT_CURSOR_DIRECTION_REVERSE);
    
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
