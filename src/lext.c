#include <lext/lext.h> // lxt_*

#include <stdlib.h> // rand, srand
#include <string.h> // memset, strncmp
#include <stddef.h> // size_t, NULL
#include <stdint.h> // int32_t
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
    LXT_KIND_RESOLUTION
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
    struct lxt_token resolution;
};

struct lxt_pattern {
    struct lxt_container containers[MAX_CONTAINERS];
    struct lxt_generator generators[MAX_GENERATORS];
    size_t container_count;
    size_t generator_count;
};

struct lxt_cursor {
    char * buffer;
    size_t offset;
    size_t length;
};

static int32_t lxt_parse(struct lxt_pattern *, char const * pattern);
static char const * lxt_parse_next(struct lxt_token *, enum lxt_kind *, char const * pattern);
static int32_t lxt_resolve_generator(struct lxt_cursor *, struct lxt_generator const *, struct lxt_pattern const *);
static int32_t lxt_resolve_variable(struct lxt_cursor *, struct lxt_token const * variable, struct lxt_pattern const *);
static int32_t lxt_resolve_container(struct lxt_cursor *, struct lxt_container const *);

static struct lxt_generator const * lxt_find_generator(struct lxt_token const *, struct lxt_pattern const *);
static struct lxt_container const * lxt_find_container(struct lxt_token const *, struct lxt_pattern const *);

static bool lxt_token_equals(struct lxt_token const *, char const * name, size_t length);

void
lxt_gen(char * const result, size_t length, char const * pattern)
{
    lxt_genk(result, length, pattern, NULL);
}

void
lxt_genk(char * const result, size_t length, char const * const pattern,
          char const * name)
{
    if (length == 0) {
        return;
    }
    
    struct lxt_pattern parsed;
    
    memset(&parsed, 0, sizeof(parsed));
    
    int32_t r = lxt_parse(&parsed, pattern);
    
    if (r != 0) {
        return;
    }
    
    srand(12345);
    
    struct lxt_generator const * generator = NULL;
    
    if (name != NULL) {
        struct lxt_token token;
        
        token.start = name;
        token.length = strlen(name);
        
        generator = lxt_find_generator(&token, &parsed);
    } else {
        generator = &parsed.generators[rand() % parsed.generator_count];
    }
    
    if (generator == NULL) {
        return;
    }
    
    char buffer[length];
    
    memset(buffer, '\0', sizeof(buffer));
    
    struct lxt_cursor cursor;
    
    cursor.buffer = buffer;
    cursor.length = length - 1; // leave 1 byte for the null-terminator
    cursor.offset = 0;
    
    if (lxt_resolve_generator(&cursor, generator, &parsed) != 0) {
        return;
    }
    
    // copy generated characters into resulting buffer
    memcpy(result, buffer, cursor.offset);
    // null-terminate the resulting buffer
    memset(result + cursor.offset, '\0', 1);
}

static
int32_t
lxt_resolve_container(struct lxt_cursor * const cursor,
                      struct lxt_container const * const container)
{
    size_t const random_index = rand() % container->entry_count;
    
    struct lxt_token const * entry = &container->entries[random_index];
    
    if (entry->length >= cursor->length - cursor->offset) {
        return -1;
    }
    
    cursor->offset += entry->length;
    
    memcpy(cursor->buffer, entry->start, entry->length);
    
    return 0;
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
struct lxt_generator const *
lxt_find_generator(struct lxt_token const * const token,
                    struct lxt_pattern const * const pattern)
{
    for (size_t i = 0; i < pattern->generator_count; i++) {
        struct lxt_generator const * const match = &pattern->generators[i];
        
        if (lxt_token_equals(token, match->entry.start, match->entry.length)) {
            return match;
        }
    }
    
    return NULL;
}

static
struct lxt_container const *
lxt_find_container(struct lxt_token const * const token,
                    struct lxt_pattern const * const pattern)
{
    for (size_t i = 0; i < pattern->container_count; i++) {
        struct lxt_container const * const match = &pattern->containers[i];
        
        if (lxt_token_equals(token, match->entry.start, match->entry.length)) {
            return match;
        }
    }
    
    return NULL;
}

static
int32_t
lxt_resolve_variable(struct lxt_cursor * const cursor,
                     struct lxt_token const * const variable,
                     struct lxt_pattern const * const pattern)
{
    struct lxt_generator const * const generator = lxt_find_generator(variable,
                                                                      pattern);
    
    if (generator != NULL) {
        return lxt_resolve_generator(cursor, generator, pattern);
    }

    struct lxt_container const * const container = lxt_find_container(variable,
                                                                      pattern);
    
    if (container == NULL) {
        return -1;
    }
    
    if (lxt_resolve_container(cursor, container) != 0) {
        return -1;
    }
    
    return 0;
}

static
int32_t
lxt_resolve_generator(struct lxt_cursor * const cursor,
                      struct lxt_generator const * const generator,
                      struct lxt_pattern const * const pattern)
{
    size_t n = 0; // constraint to stay within substring length
    
    char const * p = generator->resolution.start;
    
    struct lxt_token variable;
    
    bool token_started = false;
    bool token_ended = false;
    
    while (*p && n != generator->resolution.length) {
        if (strncmp(p, "@", 1) == 0) {
            if (token_started) {
                return -1;
            }
            
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
        
        if (token_ended) {
            token_ended = false;
            
            struct lxt_cursor tmp;
            
            tmp.buffer = cursor->buffer + cursor->offset;
            tmp.length = cursor->length - cursor->offset;
            tmp.offset = 0;
            
            if (lxt_resolve_variable(&tmp, &variable, pattern) != 0) {
                return -1;
            }
            
            cursor->offset += tmp.offset;
        }
        
        if (!token_started && !token_ended) {
            if (cursor->offset >= cursor->length) {
                return -1;
            }
            
            cursor->buffer[cursor->offset] = *p;
            cursor->offset += 1;
        }
        
        p++;
        n++;
    }
    
    if (token_started && !token_ended) {
        struct lxt_cursor tmp;
        
        tmp.buffer = cursor->buffer + cursor->offset;
        tmp.length = cursor->length - cursor->offset;
        tmp.offset = 0;
        
        if (lxt_resolve_variable(&tmp, &variable, pattern) != 0) {
            return -1;
        }
        
        cursor->offset += tmp.offset;
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
    
    token->start = NULL;
    token->length = 0;
    
    while (*pattern) {
        switch (*pattern) {
            case '(': {
                *kind = LXT_KIND_CONTAINER;
            } break;
                
            case ',':
            case ')': {
                *kind = LXT_KIND_CONTAINER_ENTRY;
            } break;
                
            case '<': {
                *kind = LXT_KIND_GENERATOR;
            } break;
                
            case '>': {
                *kind = LXT_KIND_RESOLUTION;
            } break;
                
            default:
                break;
        }
        
        if (*kind != LXT_KIND_NONE) {
            // skip trailing whitespace
            char const * p = pattern - 1;
            
            while (isspace(*p)) {
                token->length -= 1;
                
                p--;
            }
            
            return pattern + 1;
        }
        
        if (token->start == NULL && !isspace(*pattern)) {
            // skip leading whitespace
            token->start = pattern;
        }
        
        if (token->start != NULL) {
            token->length += 1;
        }
        
        pattern++;
    }
    
    return pattern;
}

static
int32_t
lxt_parse(struct lxt_pattern * const result, char const * pattern)
{
    struct lxt_container * container = NULL;
    struct lxt_generator * generator = NULL;
    
    while (*pattern) {
        if (result->container_count >= MAX_CONTAINERS) {
            return -1;
        }
        
        struct lxt_token token;
        enum lxt_kind kind;
        
        pattern = lxt_parse_next(&token, &kind, pattern);
        
        if (kind == LXT_KIND_NONE) {
            continue;
        }
        
        switch (kind) {
            case LXT_KIND_CONTAINER: {
                if (result->container_count == MAX_CONTAINERS) {
                    return -1;
                }
                
                container = &result->containers[result->container_count++];
                container->entry = token;
            } break;
            
            case LXT_KIND_CONTAINER_ENTRY: {
                if (container == NULL) {
                    return -1;
                }
                
                if (container->entry_count == MAX_CONTAINER_ENTRIES) {
                    return -1;
                }
                
                container->entries[container->entry_count++] = token;
            } break;
                
            case LXT_KIND_GENERATOR: {
                if (result->generator_count == MAX_GENERATORS) {
                    return -1;
                }
                
                generator = &result->generators[result->generator_count++];
                generator->entry = token;
            } break;
                
            case LXT_KIND_RESOLUTION: {
                if (generator == NULL) {
                    return -1;
                }
                
                generator->resolution = token;
            } break;
                
            case LXT_KIND_NONE:
                
            default:
                break;
        }
    }
    
    return 0;
}
