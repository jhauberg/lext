#include "cursor.h" // lxt_cursor, lxt_cursor_*, lxt_cursor_direction
#include "token.h" // lxt_token

#include <stdint.h> // int32_t
#include <string.h> // memcpy

#include <ctype.h> // isspace

int32_t
lxt_cursor_write(struct lxt_cursor * const cursor,
                 struct lxt_token token)
{
    if (token.length == 0) {
        return -1;
    }
    
    if (cursor->offset >= cursor->length) {
        return -1;
    }
    
    size_t const remaining_length = cursor->length - cursor->offset;
    
    if (token.length > remaining_length) {
        token.length = remaining_length;
    }
    
    memcpy(cursor->buffer + cursor->offset,
           token.start,
           token.length);
    
    cursor->offset += token.length;
    
    return 0;
}

size_t
lxt_cursor_spaces(char const * text,
                  enum lxt_cursor_direction const direction)
{
    size_t length = 0;
    int32_t offset = direction == LXT_CURSOR_DIRECTION_REVERSE ? -1 : 1;
    
    while (*text && isspace(*text)) {
        length += 1;
        
        text += offset;
    }
    
    return length;
}
