#include "template.h" // lxt_template, lxt_generator, lxt_container, lxt_*
#include "token.h" // lxt_token, lxt_token_equals
#include "rand.h" // lxt_rand32

#include <stddef.h> // size_t, NULL
#include <stdbool.h> // bool
#include <string.h> // strlen

void
lxt_get_generator(struct lxt_generator const ** generator,
                  struct lxt_template const * const template,
                  char const * const name)
{
    *generator = NULL;
    
    if (template->generator_count == 0) {
        return;
    }
    
    if (name != NULL) {
        struct lxt_token token;
        
        token.start = name;
        token.length = strlen(name);
        
        if (lxt_find_generator(generator, token, template)) {
            return;
        }
    }
    
    size_t const i = lxt_rand32(template->seed);
    
    *generator = &template->generators[i % template->generator_count];
}

bool
lxt_find_generator(struct lxt_generator const ** const generator,
                   struct lxt_token const token,
                   struct lxt_template const * const template)
{
    for (size_t i = 0; i < template->generator_count; i++) {
        struct lxt_generator const * const match = &template->generators[i];
        
        if (lxt_token_equals(token, match->entry)) {
            *generator = match;
            
            return true;
        }
    }
    
    return false;
}

bool
lxt_find_container(struct lxt_container const ** const container,
                   struct lxt_token const token,
                   struct lxt_template const * const template)
{
    for (size_t i = 0; i < template->container_count; i++) {
        struct lxt_container const * const match = &template->containers[i];
        
        if (lxt_token_equals(token, match->entry)) {
            *container = match;
            
            return true;
        }
    }
    
    return false;
}

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
