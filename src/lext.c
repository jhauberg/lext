#include <lext/lext.h> // lxt_opts, lxt_gen

#include "template.h" // lxt_template, lxt_container, lxt_generator
#include "token.h" // lxt_token, lxt_kind, lxt_token_*
#include "cursor.h" // lxt_cursor, lxt_cursor_*
#include "rand.h" // lxt_rand32

#include <string.h> // memset
#include <stddef.h> // size_t, NULL
#include <stdint.h> // int32_t, uint32_t

extern inline uint32_t lxt_rand32(uint32_t * seed);

/**
 * Parse a LEXT pattern into a template.
 */
static int32_t lxt_parse(struct lxt_template *,
                         char const * pattern);
/**
 * Parse the current token and return a pointer to the next.
 *
 * This function reads bytes up until reaching a keyword character, indicating
 * how to interpret either the bytes already read, or the bytes to come.
 *
 * This is important to keep in mind, as it may seem counter-intuitive.
 *
 * For example, reading "abcd" immediately followed by an angle bracket ("<")
 * would parse "abcd" as a generator token and discard the "<".
 * The counter-intuitive issue here is that, in this case, the "<" does *not*
 * represent the *beginning* of a generator sequence token, but rather signal
 * the *end* of a generator token.
 *
 * An exception to this rule is comment tokens which are indicated
 * by a starting keyword character ("#").
 */
static char const * lxt_parse_token(struct lxt_token *,
                                    enum lxt_kind *,
                                    char const * pattern);
/**
 * Parse the current sequence token and return a pointer to the next.
 *
 * Unlike `lxt_parse_token`, this function reads bytes and implicitly consider
 * each byte read a text token.
 *
 * This is the case until a variable keyword character is encountered (or
 * end of sequence), at which point any following bytes read, until reaching a
 * whitespace character, are considered part of a variable token.
 */
static char const * lxt_parse_sequence(struct lxt_token *,
                                       enum lxt_kind *,
                                       char const * sequence,
                                       char const * end);

/**
 * Read the next bytes as a particular kind of token and return a pointer to
 * the end.
 */
static char const * lxt_read_token(struct lxt_token * token,
                                   enum lxt_kind kind,
                                   char const * pattern);
/**
 * Read bytes from the start of a token up to, but not including, any specified
 * delimiting character.
 */
static char const * lxt_read_up_to(struct lxt_token *,
                                   char const delimiters[],
                                   size_t delimiter_count);

static int32_t lxt_process_token(struct lxt_template *,
                                 struct lxt_token,
                                 enum lxt_kind);

static int32_t lxt_resolve_generator(struct lxt_cursor *,
                                     struct lxt_generator const *,
                                     struct lxt_template const *);
static int32_t lxt_resolve_variable(struct lxt_cursor *,
                                    struct lxt_token,
                                    struct lxt_template const *);

struct lxt_opts const LXT_OPTS_NONE = {
    .generator = NULL,
    .seed = NULL
};

enum lxt_error
lxt_gen(char * const buffer,
        size_t const length,
        char const * const pattern,
        struct lxt_opts options)
{
    struct lxt_template template;
    
    memset(&template, 0, sizeof(template));
    
    if (lxt_parse(&template, pattern) != 0) {
        return LXT_ERROR_INVALID_TEMPLATE;
    }
    
    uint32_t default_seed = 2147483647;
    
    template.seed = &default_seed;
    
    if (options.seed != NULL) {
        template.seed = options.seed;
    }
    
    struct lxt_generator const * generator = NULL;
    
    lxt_get_generator(&generator, &template, options.generator);
    
    if (generator == NULL) {
        return LXT_ERROR_GENERATOR_NOT_FOUND;
    }
    
    struct lxt_cursor cursor;
    
    cursor.buffer = buffer;
    cursor.length = length - 1; // leave 1 byte for the null-terminator
    cursor.offset = 0;
    
    if (lxt_resolve_generator(&cursor, generator, &template) != 0) {
        // something went wrong
    }
    
    // null-terminate the resulting buffer
    memset(buffer + cursor.offset, '\0', 1);
    
    return LXT_ERROR_NONE;
}

static
int32_t
lxt_parse(struct lxt_template * const template,
          char const * pattern)
{
    while (*pattern) {
        struct lxt_token token;
        enum lxt_kind kind;

        pattern = lxt_parse_token(&token, &kind, pattern);
        
        if (kind != LXT_KIND_NONE &&
            kind != LXT_KIND_COMMENT) {
            lxt_token_trim(&token);
        }
        
        if (token.length == 0) {
            continue;
        }
        
        if (!lxt_token_validates(token, kind)) {
            return -1;
        }
        
        if (lxt_process_token(template, token, kind) != 0) {
            return -1;
        }
    }
    
    return 0;
}

static
char const *
lxt_read_up_to(struct lxt_token * const token,
               char const delimiters[],
               size_t const delimiter_count)
{
    char const * p = token->start;
    
    while (*p) {
        char const c = *p;

        for (size_t i = 0; i < delimiter_count; i++) {
            char const delimiter = delimiters[i];
            
            // todo: unless preceded by backslash?
            if (c == delimiter) {
                return p;
            }
        }
        
        token->length += 1;
        
        p++;
    }
    
    return p;
}

static
char const *
lxt_read_token(struct lxt_token * const token,
               enum lxt_kind const kind,
               char const * pattern)
{
    switch (kind) {
        case LXT_KIND_COMMENT: {
            token->start = pattern;
            
            char delimiters[1] = { '\n' };
            
            return lxt_read_up_to(token, delimiters, 1);
        }
            
        case LXT_KIND_CONTAINER_ENTRY: {
            token->start = pattern + 1;
            
            char delimiters[2] = { ')', ',' };
            
            return lxt_read_up_to(token, delimiters, 2);
        }
            
        case LXT_KIND_SEQUENCE: {
            token->start = pattern + 1;
            
            char delimiters[1] = { '>' };
            
            return lxt_read_up_to(token, delimiters, 1);
        }
            
        case LXT_KIND_NONE: {
            token->start = pattern;
            token->length += 1;
            
            return pattern + token->length;
        }
            
        case LXT_KIND_CONTAINER:
        case LXT_KIND_GENERATOR:
        case LXT_KIND_VARIABLE:
        case LXT_KIND_TEXT:
            /* fall through */
            break;
    }
    
    return NULL;
}

static
char const *
lxt_parse_token(struct lxt_token * const token,
                enum lxt_kind * const kind,
                char const * pattern)
{
    *kind = LXT_KIND_NONE;

    token->start = NULL;
    token->length = 0;
    
    if (lxt_token_starts(kind, pattern)) {
        return lxt_read_token(token, *kind, pattern);
    }
    
    token->start = pattern;
    
    while (*pattern) {
        token->length += 1;
        
        if (lxt_token_ends(kind, pattern)) {
            return pattern + 1;
        }
        
        pattern++;
    }
    
    return pattern;
}

static
char const *
lxt_parse_sequence(struct lxt_token * const token,
                   enum lxt_kind * const kind,
                   char const * sequence,
                   char const * const end)
{
    *kind = LXT_KIND_NONE;
    
    token->length = 0;
    token->start = NULL;
    
    while (*sequence && sequence != end) {
        if (token->start == NULL && *sequence == VARIABLE_CHARACTER) {
            // skip this character
            // todo: unless preceded by backslash?
            sequence++;
            
            // begin parsing variable
            token->start = sequence;
            
            *kind = LXT_KIND_VARIABLE;
        }
        
        if (token->start != NULL && !lxt_token_character(*sequence)) {
            // end parsing variable
            return sequence;
        }
        
        token->length += 1;
        
        if (token->start == NULL) {
            // parse as a chunk of text
            token->start = sequence;
            
            *kind = LXT_KIND_TEXT;
            
            return sequence + 1;
        }
        
        sequence++;
    }
    
    return sequence;
}

static
int32_t
lxt_process_token(struct lxt_template * const template,
                  struct lxt_token token,
                  enum lxt_kind const kind)
{
    switch (kind) {
        case LXT_KIND_CONTAINER: {
            if (lxt_append_container(template, token) != 0) {
                return -1;
            }
        } break;
            
        case LXT_KIND_CONTAINER_ENTRY: {
            if (lxt_append_container_entry(template, token) != 0) {
                return -1;
            }
        } break;
            
        case LXT_KIND_GENERATOR: {
            if (lxt_append_generator(template, token) != 0) {
                return -1;
            }
        } break;
            
        case LXT_KIND_SEQUENCE: {
            if (lxt_append_sequence(template, token) != 0) {
                return -1;
            }
        } break;
        
        case LXT_KIND_VARIABLE:
        case LXT_KIND_TEXT:
        case LXT_KIND_COMMENT:
        case LXT_KIND_NONE:
            /* fall through */
            break;
    }
    
    return 0;
}

static
int32_t
lxt_resolve_generator(struct lxt_cursor * const cursor,
                      struct lxt_generator const * const gen,
                      struct lxt_template const * const template)
{
    char const * next = gen->sequence.start;
    char const * const end = next + gen->sequence.length;
    
    while (*next && next != end) {
        struct lxt_token token;
        enum lxt_kind kind;
        
        next = lxt_parse_sequence(&token, &kind, next, end);
        
        if (kind == LXT_KIND_NONE) {
            continue;
        }
        
        if (token.length == 0) {
            // zero-length token will neither resolve as variable nor
            // point to writable content; skip it
            continue;
        }
        
        if (kind == LXT_KIND_VARIABLE) {
            if (lxt_token_equals(token, gen->entry)) {
                // variable points to its own generator; skip it or
                // incur the wrath of infinite recursion
                continue;
            }
            
            if (lxt_resolve_variable(cursor, token, template) != 0) {
                return -1;
            }
        } else if (kind == LXT_KIND_TEXT) {
            if (lxt_cursor_write(cursor, token) != 0) {
                return -1;
            }
        }
    }
    
    return 0;
}

static
int32_t
lxt_resolve_variable(struct lxt_cursor * const cursor,
                     struct lxt_token const variable,
                     struct lxt_template const * const template)
{
    struct lxt_generator const * generator = NULL;
    
    if (lxt_find_generator(&generator, variable, template)) {
        if (lxt_resolve_generator(cursor, generator, template) != 0) {
            return -1;
        }
        
        return 0;
    }
    
    struct lxt_container const * container = NULL;
    
    if (!lxt_find_container(&container, variable, template)) {
        return -1;
    }
    
    if (container->entry_count == 0) {
        // resolve by doing nothing
        return 0;
    }
    
    size_t const i = lxt_rand32(template->seed) % container->entry_count;
    
    struct lxt_token const * entry = &container->entries[i];
    
    if (lxt_cursor_write(cursor, *entry) != 0) {
        return -1;
    }
    
    return 0;
}
