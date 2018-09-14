#include <lext/lext.h> // lxt_opts, lxt_gen

#include <string.h> // memset, strncmp
#include <stddef.h> // size_t, NULL
#include <stdint.h> // int32_t, uint32_t
#include <stdbool.h> // bool

#include <ctype.h> // isspace

#define MAX_CONTAINERS (64)
#define MAX_GENERATORS (64)

#define MAX_CONTAINER_ENTRIES (128)

#define VARIABLE_CHARACTER '@'
#define COMMENT_CHARACTER '#'

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

enum lxt_direction {
    LXT_DIRECTION_FORWARD,
    LXT_DIRECTION_REVERSE
};

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

struct lxt_container {
    struct lxt_token entries[MAX_CONTAINER_ENTRIES];
    struct lxt_token entry;
    size_t entry_count;
};

struct lxt_generator {
    struct lxt_token entry;
    struct lxt_token sequence;
};

struct lxt_template {
    struct lxt_container containers[MAX_CONTAINERS];
    struct lxt_generator generators[MAX_GENERATORS];
    uint32_t * seed;
    size_t container_count;
    size_t generator_count;
};

/**
 * Represents a writable buffer.
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

static uint32_t lxt_rand32(uint32_t * seed);

/**
 * Get a pointer to a generator by name.
 *
 * If name is NULL, gets a random generator.
 */
static void lxt_get_generator(struct lxt_generator const **,
                              struct lxt_template const *,
                              char const * name);

static int32_t lxt_parse(struct lxt_template *,
                         char const * pattern);
/**
 * Parse the current template token and return a pointer to the next.
 *
 * This function reads bytes up until reaching a keyword character,
 * indicating how to interpret the read bytes.
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
 * Determine whether a character matches a token indicator.
 */
static bool lxt_is_keyword(char character, enum lxt_kind *);

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
static void lxt_trim_token(struct lxt_token *);

static int32_t lxt_process_token(struct lxt_template *,
                                 struct lxt_token,
                                 enum lxt_kind);

static int32_t lxt_append_container(struct lxt_template *,
                                    struct lxt_token);
static int32_t lxt_append_container_entry(struct lxt_template *,
                                          struct lxt_token);
static int32_t lxt_append_generator(struct lxt_template *,
                                    struct lxt_token);
static int32_t lxt_append_sequence(struct lxt_template *,
                                   struct lxt_token);

static int32_t lxt_resolve_generator(struct lxt_cursor *,
                                     struct lxt_generator const *,
                                     struct lxt_template const *);
static int32_t lxt_resolve_variable(struct lxt_cursor *,
                                    struct lxt_token const *,
                                    struct lxt_template const *);

static int32_t lxt_write_token(struct lxt_cursor *,
                               struct lxt_token const *);

static bool lxt_find_generator(struct lxt_generator const **,
                               struct lxt_token const *,
                               struct lxt_template const *);
static bool lxt_find_container(struct lxt_container const **,
                               struct lxt_token const *,
                               struct lxt_template const *);

static bool lxt_token_equals(struct lxt_token const *,
                             char const * name,
                             size_t length);

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
static size_t lxt_count_space(char const * text,
                              enum lxt_direction);

enum lxt_result
lxt_gen(char * const buffer,
        size_t const length,
        char const * const pattern,
        struct lxt_opts options)
{
    struct lxt_template template;
    
    memset(&template, 0, sizeof(template));
    
    if (lxt_parse(&template, pattern) != 0) {
        return LXT_RESULT_INVALID_TEMPLATE;
    }
    
    uint32_t default_seed = 2147483647;
    
    template.seed = &default_seed;
    
    if (options.seed != NULL) {
        template.seed = options.seed;
    }
    
    struct lxt_generator const * generator = NULL;
    
    lxt_get_generator(&generator, &template, options.generator);
    
    if (generator == NULL) {
        return LXT_RESULT_GENERATOR_NOT_FOUND;
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
    
    return LXT_RESULT_GENERATED;
}

static
uint32_t
lxt_rand32(uint32_t * const seed)
{
    uint32_t x = *seed;
    
    x ^= x << 13;
    x ^= (x & 0xffffffffUL) >> 17;
    x ^= x << 5;
    
    *seed = x;
    
    return (x & 0xffffffffUL);
}

static
void
lxt_get_generator(struct lxt_generator const ** generator,
                  struct lxt_template const * const template,
                  char const * const name)
{
    if (template->generator_count == 0) {
        return;
    }
    
    if (name != NULL) {
        struct lxt_token token;
        
        token.start = name;
        token.length = strlen(name);
        
        if (lxt_find_generator(generator, &token, template)) {
            return;
        }
    }
    
    size_t const i = lxt_rand32(template->seed);
    
    *generator = &template->generators[i % template->generator_count];
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
        
        if (kind == LXT_KIND_NONE ||
            kind == LXT_KIND_COMMENT) {
            continue;
        }
        
        if (lxt_process_token(template, token, kind) != 0) {
            return -1;
        }
    }
    
    return 0;
}

static
char const *
lxt_parse_token(struct lxt_token * const token,
                enum lxt_kind * const kind,
                char const * pattern)
{
    *kind = LXT_KIND_NONE;
    
    token->start = pattern;
    token->length = 0;
    
    while (*pattern) {
        enum lxt_kind token_kind;
        
        if (lxt_is_keyword(*pattern, &token_kind)) {
            *kind = token_kind;
            
            // skip the keyword character
            return pattern + 1;
        }
        
        // look ahead and determine whether a comment is about to be initiated
        if (*(pattern + 1) == COMMENT_CHARACTER) {
            // include current character as part of current token
            token->length += 1;
            // and skip it (jumping to start of comment)
            return pattern + 1;
        }
        
        if (*pattern == COMMENT_CHARACTER) {
            *kind = LXT_KIND_COMMENT;
        } else if (*kind == LXT_KIND_COMMENT) {
            if (*pattern == '\n') {
                // comment ended; skip character
                return pattern + 1;
            }
        }
        
        token->length += 1;
        
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
            sequence++;
            
            token->start = sequence;
            
            *kind = LXT_KIND_VARIABLE;
        }
        
        if (token->start != NULL && isspace(*sequence)) {
            return sequence;
        }
        
        if (token->start != NULL) {
            token->length += 1;
        } else {
            token->start = sequence;
            token->length = 1;
            
            *kind = LXT_KIND_TEXT;
            
            return sequence + 1;
        }
        
        sequence++;
    }
    
    return sequence;
}

static
bool
lxt_is_keyword(char const character,
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
    
    if (*kind != LXT_KIND_NONE) {
        return true;
    }
    
    return false;
}

static
void
lxt_trim_token(struct lxt_token * const token)
{
    size_t const leading = lxt_count_space(token->start,
                                           LXT_DIRECTION_FORWARD);
    
    token->start = token->start + leading;
    token->length -= leading;
    
    size_t const trailing = lxt_count_space(token->start + token->length - 1,
                                            LXT_DIRECTION_REVERSE);
    
    token->length -= trailing;
}

static
int32_t
lxt_process_token(struct lxt_template * const template,
                  struct lxt_token token,
                  enum lxt_kind const kind)
{
    if (kind != LXT_KIND_NONE) {
        lxt_trim_token(&token);
    }
    
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
lxt_append_container(struct lxt_template * const template,
                     struct lxt_token const token)
{
    if (template->container_count == MAX_CONTAINERS) {
        return -1;
    }
    
    size_t const cur_index = template->container_count;
    
    struct lxt_container * const container = &template->containers[cur_index];
    
    container->entry = token;
    
    template->container_count += 1;
    
    return 0;
}

static
int32_t
lxt_append_container_entry(struct lxt_template * const template,
                           struct lxt_token const token)
{
    if (template->container_count == 0) {
        return -1;
    }
    
    size_t const cur_index = template->container_count - 1;
    
    struct lxt_container * const container = &template->containers[cur_index];
    
    if (container->entry_count == MAX_CONTAINER_ENTRIES) {
        return -1;
    }
    
    container->entries[container->entry_count] = token;
    container->entry_count += 1;
    
    return 0;
}

static
int32_t
lxt_append_generator(struct lxt_template * const template,
                     struct lxt_token const token)
{
    if (template->generator_count == MAX_GENERATORS) {
        return -1;
    }
    
    size_t const cur_index = template->generator_count;
    
    struct lxt_generator * const generator = &template->generators[cur_index];
    
    generator->entry = token;
    
    template->generator_count += 1;
    
    return 0;
}

static
int32_t
lxt_append_sequence(struct lxt_template * const template,
                    struct lxt_token const token)
{
    if (template->container_count == 0) {
        return -1;
    }
    
    size_t const cur_index = template->generator_count - 1;
    
    struct lxt_generator * const generator = &template->generators[cur_index];
    
    generator->sequence = token;
    
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
            if (token.length == gen->entry.length &&
                strncmp(token.start, gen->entry.start, token.length) == 0) {
                // variable points to its own generator; skip it or
                // incur the wrath of infinite recursion
                continue;
            }
            
            if (lxt_resolve_variable(cursor, &token, template) != 0) {
                return -1;
            }
        } else if (kind == LXT_KIND_TEXT) {
            if (lxt_write_token(cursor, &token) != 0) {
                return -1;
            }
        }
    }
    
    return 0;
}

static
int32_t
lxt_resolve_variable(struct lxt_cursor * const cursor,
                     struct lxt_token const * const variable,
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
    
    size_t const i = lxt_rand32(template->seed) % container->entry_count;
    
    struct lxt_token const * entry = &container->entries[i];
    
    if (lxt_write_token(cursor, entry) != 0) {
        return -1;
    }
    
    return 0;
}

static
int32_t
lxt_write_token(struct lxt_cursor * const cursor,
                struct lxt_token const * const entry)
{
    if (cursor->offset >= cursor->length ||
        entry->length >= cursor->length - cursor->offset) {
        return -1;
    }
    
    memcpy(cursor->buffer + cursor->offset,
           entry->start,
           entry->length);
    
    cursor->offset += entry->length;
    
    return 0;
}

static
bool
lxt_find_generator(struct lxt_generator const ** const generator,
                   struct lxt_token const * const token,
                   struct lxt_template const * const template)
{
    for (size_t i = 0; i < template->generator_count; i++) {
        struct lxt_generator const * const match = &template->generators[i];
        
        if (lxt_token_equals(token, match->entry.start, match->entry.length)) {
            *generator = match;
            
            return true;
        }
    }
    
    return false;
}

static
bool
lxt_find_container(struct lxt_container const ** const container,
                   struct lxt_token const * const token,
                   struct lxt_template const * const template)
{
    for (size_t i = 0; i < template->container_count; i++) {
        struct lxt_container const * const match = &template->containers[i];
        
        if (lxt_token_equals(token, match->entry.start, match->entry.length)) {
            *container = match;
            
            return true;
        }
    }
    
    return false;
}

static
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

static
size_t
lxt_count_space(char const * text,
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

