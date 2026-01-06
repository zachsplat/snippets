#ifndef ARENA_H
#define ARENA_H

/* bump allocator. allocate a big chunk, hand out pieces.
   free everything at once when done. no individual free. */

#include <stdlib.h>
#include <stdint.h>

struct arena {
    uint8_t *base;
    size_t size;
    size_t used;
};

static inline struct arena *arena_new(size_t size) {
    struct arena *a = malloc(sizeof(*a));
    a->base = malloc(size);
    a->size = size;
    a->used = 0;
    return a;
}

static inline void *arena_alloc(struct arena *a, size_t n) {
    /* align to 8 */
    n = (n + 7) & ~(size_t)7;
    if (a->used + n > a->size) return NULL;
    void *p = a->base + a->used;
    a->used += n;
    return p;
}

static inline void arena_reset(struct arena *a) {
    a->used = 0;
}

static inline void arena_free(struct arena *a) {
    free(a->base);
    free(a);
}

#endif
