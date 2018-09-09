#include <lext/lext.h> // lxt_*

#include <string.h> // memset, strncmp
#include <stddef.h> // size_t, NULL
#include <stdint.h> // int32_t, uint32_t
#include <stdbool.h> // bool

#include <ctype.h> // isspace

#define MAX_CONTAINERS (64)
#define MAX_GENERATORS (64)

#define MAX_CONTAINER_ENTRIES (128)

enum lxt_kind {
    LXT_KIND_NONE,
    LXT_KIND_CONTAINER,
    LXT_KIND_CONTAINER_ENTRY,
    LXT_KIND_GENERATOR,
    LXT_KIND_SEQUENCE
};

enum lxt_direction {
    LXT_DIRECTION_FORWARD,
    LXT_DIRECTION_BACKWARD
};

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

struct lxt_cursor {
    char * buffer;
    size_t offset;
    size_t length;
};

static uint32_t lxt_rand32(uint32_t * seed);

static void lxt_get_generator(struct lxt_generator const **,
                              struct lxt_template const *,
                              char const * name);

static int32_t lxt_parse(struct lxt_template *,
                         char const * pattern);
static char const * lxt_parse_next(struct lxt_token *,
                                   enum lxt_kind *,
                                   char const * pattern);

static bool lxt_is_keyword(char character, enum lxt_kind *);

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

static int32_t lxt_write(struct lxt_cursor *, char);
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

static size_t lxt_count_space(char const * text,
                              enum lxt_direction);

void
lxt_gen(char * const result,
        size_t const length,
        char const * const pattern,
        struct lxt_opts options)
{
    struct lxt_template template;
    
    memset(&template, 0, sizeof(template));
    
    if (lxt_parse(&template, pattern) != 0) {
        return;
    }
    
    uint32_t default_seed = 2147483647;
    
    template.seed = &default_seed;
    
    if (options.seed != NULL) {
        template.seed = options.seed;
    }
    
    struct lxt_generator const * generator = NULL;
    
    lxt_get_generator(&generator, &template, options.generator);
    
    if (generator == NULL) {
        return;
    }
    
    struct lxt_cursor cursor;
    
    cursor.buffer = result;
    cursor.length = length - 1; // leave 1 byte for the null-terminator
    cursor.offset = 0;
    
    if (lxt_resolve_generator(&cursor, generator, &template) != 0) {
        // something went wrong
    }
    
    // null-terminate the resulting buffer
    memset(result + cursor.offset, '\0', 1);
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
        
        pattern = lxt_parse_next(&token, &kind, pattern);
        
        if (kind == LXT_KIND_NONE) {
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
lxt_parse_next(struct lxt_token * const token,
               enum lxt_kind * const kind,
               char const * pattern)
{
    *kind = LXT_KIND_NONE;
    
    token->start = pattern;
    token->length = 0;
    
    while (*pattern) {
        if (lxt_is_keyword(*pattern, kind)) {
            // skip this character
            return pattern + 1;
        }
        
        if (token->start != NULL) {
            token->length += 1;
        }
        
        pattern++;
    }
    
    return pattern;
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
    // trim trailing whitespace
    size_t const trailing = lxt_count_space(token->start + token->length,
                                            LXT_DIRECTION_BACKWARD);
    
    token->length -= trailing;
    
    // trim leading whitespace
    size_t const leading = lxt_count_space(token->start,
                                           LXT_DIRECTION_FORWARD);
    
    token->start = token->start + leading;
    token->length -= leading;
}

static
int32_t
lxt_process_token(struct lxt_template * const template,
                  struct lxt_token token,
                  enum lxt_kind const kind)
{
    lxt_trim_token(&token);
    
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
            
        case LXT_KIND_NONE:
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
                      struct lxt_generator const * const generator,
                      struct lxt_template const * const template)
{
    size_t n = 0; // constraint to stay within substring length
    
    char const * p = generator->sequence.start;
    
    struct lxt_token variable = {
        .start = NULL,
        .length = 0
    };
    
    bool token_started = false;
    bool token_ended = false;
    
    while (*p && n != generator->sequence.length) {
        if (strncmp(p, "@", 1) == 0 && !token_started) {
            if (isspace(*(p + 1))) {
                // immediately followed by whitespace
                return -1;
            }
            
            // skip this character
            p++;
            n++;
            
            variable.start = p;
            variable.length = 0;
            
            token_started = true;
        }
        
        if (token_started && isspace(*p)) {
            token_started = false;
            token_ended = true;
        }
        
        if (token_started && !token_ended) {
            variable.length += 1;
        }
        
        if (token_started && n + 1 == generator->sequence.length) {
            // sequence ends next iteration, but token must still be resolved
            token_started = false;
            token_ended = true;
        }
        
        if (token_ended) {
            token_ended = false;
            
            if (lxt_resolve_variable(cursor, &variable, template) != 0) {
                return -1;
            }
        }
        
        if (!token_started && !token_ended) {
            if (lxt_write(cursor, *p) != 0) {
                return -1;
            }
        }
        
        p++;
        n++;
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
lxt_write(struct lxt_cursor * const cursor,
          char const character)
{
    char const buffer[2] = {
        character, '\0'
    };
    
    struct lxt_token token;
    
    token.start = buffer;
    token.length = 1;
    
    return lxt_write_token(cursor, &token);
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
    int32_t offset = 1;
    
    if (direction == LXT_DIRECTION_BACKWARD) {
        text = text - 1;
        offset = -1;
    }
    
    while (isspace(*text)) {
        length += 1;
        
        text += offset;
    }
    
    return length;
}

