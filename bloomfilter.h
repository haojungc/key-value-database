#ifndef BLOOMFILTER_H
#define BLOOMFILTER_H
#include <stdint.h>

typedef struct bloomfilter {
    /* Used for loading/saving the bloom filter */
    char *state_file;
    /* Loads the bloom filter from filepath. */
    void (*load)(const char *filepath);
    /* Saves the current bloom filter. */
    void (*save)();
    /* Frees the memory allocated for the bloom filter. */
    void (*free)();
    /* Sets k entries in the bloom filter to 1.
     * k: the number of hash functions */
    void (*add)(const uint64_t key);
    /* Checks if a given key is in the database by looking up the bloom filter.
     * Returns 0 if the key is in the database, otherwise returns -1. */
    int_fast8_t (*lookup)(const uint64_t key);
} bloomfilter_t;

/* Initializes the bloom filter. */
void init_bloomfilter(bloomfilter_t *bf);

#endif