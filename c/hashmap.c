/* minimal open-addressing hashmap. string keys, void* values.
   robin hood hashing because why not */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INITIAL_CAP 16
#define LOAD_FACTOR 0.75

struct hm_entry {
    char *key;
    void *val;
    int psl;  /* probe sequence length */
};

struct hashmap {
    struct hm_entry *buckets;
    int cap;
    int len;
};

static unsigned int hash_str(const char *s) {
    unsigned int h = 5381;
    while (*s)
        h = ((h << 5) + h) + (unsigned char)*s++;
    return h;
}

struct hashmap *hm_new(void) {
    struct hashmap *m = calloc(1, sizeof(*m));
    m->cap = INITIAL_CAP;
    m->buckets = calloc(m->cap, sizeof(struct hm_entry));
    return m;
}

static void hm_grow(struct hashmap *m);

void hm_set(struct hashmap *m, const char *key, void *val) {
    if ((float)m->len / m->cap > LOAD_FACTOR)
        hm_grow(m);

    struct hm_entry e = { .key = strdup(key), .val = val, .psl = 0 };
    unsigned int idx = hash_str(key) % m->cap;

    for (;;) {
        if (!m->buckets[idx].key) {
            m->buckets[idx] = e;
            m->len++;
            return;
        }
        if (strcmp(m->buckets[idx].key, key) == 0) {
            m->buckets[idx].val = val;
            free(e.key);
            return;
        }
        /* robin hood: swap if current entry has shorter probe */
        if (e.psl > m->buckets[idx].psl) {
            struct hm_entry tmp = m->buckets[idx];
            m->buckets[idx] = e;
            e = tmp;
        }
        e.psl++;
        idx = (idx + 1) % m->cap;
    }
}

void *hm_get(struct hashmap *m, const char *key) {
    unsigned int idx = hash_str(key) % m->cap;
    int psl = 0;
    while (m->buckets[idx].key) {
        if (psl > m->buckets[idx].psl)
            return NULL;
        if (strcmp(m->buckets[idx].key, key) == 0)
            return m->buckets[idx].val;
        psl++;
        idx = (idx + 1) % m->cap;
    }
    return NULL;
}

static void hm_grow(struct hashmap *m) {
    int old_cap = m->cap;
    struct hm_entry *old = m->buckets;
    m->cap *= 2;
    m->buckets = calloc(m->cap, sizeof(struct hm_entry));
    m->len = 0;
    for (int i = 0; i < old_cap; i++) {
        if (old[i].key) {
            hm_set(m, old[i].key, old[i].val);
            free(old[i].key);  /* hm_set strdup's */
        }
    }
    free(old);
}

void hm_free(struct hashmap *m) {
    for (int i = 0; i < m->cap; i++)
        free(m->buckets[i].key);
    free(m->buckets);
    free(m);
}

#ifdef TEST_HASHMAP
int main(void) {
    struct hashmap *m = hm_new();
    hm_set(m, "foo", "bar");
    hm_set(m, "baz", "qux");
    printf("foo=%s\n", (char *)hm_get(m, "foo"));
    printf("baz=%s\n", (char *)hm_get(m, "baz"));
    printf("miss=%p\n", hm_get(m, "nope"));
    hm_free(m);
}
#endif
