#pragma once

#include "token.h" // lxt_token :completeness

#include <stddef.h> // size_t
#include <stdint.h> // uint32_t, int32_t
#include <stdbool.h> // bool

#define MAX_CONTAINERS (64)
#define MAX_GENERATORS (64)

#define MAX_CONTAINER_ENTRIES (128)

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
 * Get a pointer to a generator by name.
 *
 * If name is NULL, gets a random generator.
 */
void lxt_get_generator(struct lxt_generator const **,
                       struct lxt_template const *,
                       char const * name);

bool lxt_find_generator(struct lxt_generator const **,
                        struct lxt_token,
                        struct lxt_template const *);
bool lxt_find_container(struct lxt_container const **,
                        struct lxt_token,
                        struct lxt_template const *);

int32_t lxt_append_container(struct lxt_template *,
                             struct lxt_token);
int32_t lxt_append_container_entry(struct lxt_template *,
                                   struct lxt_token);
int32_t lxt_append_generator(struct lxt_template *,
                             struct lxt_token);
int32_t lxt_append_sequence(struct lxt_template *,
                            struct lxt_token);
