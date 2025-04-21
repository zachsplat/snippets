#ifndef BITSET_H
#define BITSET_H

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

/* simple fixed-size bitset */

typedef struct {
    uint64_t *words;
    int nwords;
} bitset_t;

static inline bitset_t *bitset_new(int nbits) {
    bitset_t *bs = malloc(sizeof(bitset_t));
    bs->nwords = (nbits + 63) / 64;
    bs->words = calloc(bs->nwords, sizeof(uint64_t));
    return bs;
}

static inline void bitset_free(bitset_t *bs) {
    free(bs->words);
    free(bs);
}

static inline void bitset_set(bitset_t *bs, int i) {
    bs->words[i / 64] |= (1ULL << (i % 64));
}

static inline void bitset_clear(bitset_t *bs, int i) {
    bs->words[i / 64] &= ~(1ULL << (i % 64));
}

static inline int bitset_test(bitset_t *bs, int i) {
    return (bs->words[i / 64] >> (i % 64)) & 1;
}

static inline int bitset_popcount(bitset_t *bs) {
    int count = 0;
    for (int i = 0; i < bs->nwords; i++)
        count += __builtin_popcountll(bs->words[i]);
    return count;
}

#endif
