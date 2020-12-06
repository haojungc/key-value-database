#include "bloomfilter.h"
#include "utils.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static char state_file[] = "bf.state";
static uint64_t *bit64;
static const size_t bloom_filter_size = 0x1ULL << 31; /* 2^31 bits */

/* static function prototypes */
/* Loads the bloom filter from filepath. */
static void load(const char *filepath);
/* Saves the current bloom filter. */
static void save(const char *filepath);
/* Frees the memory allocated for the bloom filter. */
static void free_memory();
/* Sets k entries in the bloom filter to 1.
 * k: the number of hash functions */
static void add(const uint64_t key);
/* Checks if a given key is in the database by looking up the bloom filter.
 * Returns 0 if the key is in the database, otherwise returns -1. */
static int_fast8_t lookup(const uint64_t key);
static uint32_t hash(const uint64_t key, const uint64_t a, const uint64_t b);

/* static functions */
static void load(const char *filepath) {
    puts("loading bloom filter ...");
    FILE *fp = safe_fopen(filepath, "rb");
    safe_fread(bit64, sizeof(uint64_t), bloom_filter_size >> 6, fp);
    fclose(fp);
}

static void save(const char *filepath) {
    puts("saving bloom filter ...");
    /* TODO:
     * reduce the overhead: http://www.cplusplus.com/reference/cstdio/rewind/ */
    FILE *fp = safe_fopen(filepath, "wb");
    safe_fwrite(bit64, sizeof(uint64_t), bloom_filter_size >> 6, fp);
    fclose(fp);
}

static void free_memory() { free(bit64); }

static void add(const uint64_t key) {
    uint32_t h, index;
    uint_fast8_t shift;

    /* hash 1 */
    h = hash(key, 31, 1150616525);
    index = h >> 6;
    shift = h - (index << 6);
    // printf("index: %8u, shift: %2u\n", index, shift);
    bit64[index] |= (0x1ULL << shift);

    /* hash 2 */
    h = hash(key, 23, 572251735);
    index = h >> 6;
    shift = h - (index << 6);
    // printf("index: %8u, shift: %2u\n", index, shift);
    bit64[index] |= (0x1ULL << shift);

    /* hash 3 */
    h = hash(key, 47, 258054038);
    index = h >> 6;
    shift = h - (index << 6);
    // printf("index: %8u, shift: %2u\n", index, shift);
    bit64[index] |= (0x1ULL << shift);
}

static int_fast8_t lookup(const uint64_t key) {
    int_fast8_t is_in_database = 1;
    uint32_t h, index;
    uint_fast8_t shift;

    /* hash 1 */
    h = hash(key, 31, 1150616525);
    index = h >> 6;
    shift = h - (index << 6);
    is_in_database &= ((bit64[index] >> shift) & 0x1);
    if (!is_in_database)
        return -1;

    /* hash 2 */
    h = hash(key, 23, 572251735);
    index = h >> 6;
    shift = h - (index << 6);
    is_in_database &= ((bit64[index] >> shift) & 0x1);
    if (!is_in_database)
        return -1;

    /* hash 3 */
    h = hash(key, 47, 258054038);
    index = h >> 6;
    shift = h - (index << 6);
    is_in_database &= ((bit64[index] >> shift) & 0x1);

    /* Returns 0 if the key is found, otherwise returns -1 */
    return is_in_database ? 0 : -1;
}

static uint32_t hash(const uint64_t key, const uint64_t a, const uint64_t b) {
    uint64_t left = key >> 32;
    uint64_t right = key & UINT32_MAX;
    left = (uint64_t)(a * left + b);
    right = (uint64_t)(a * right + b);
    return (left ^ right) & INT32_MAX;
}

/* extern functions */
void init_bloomfilter(bloomfilter_t *bf) {
    puts("initializing bloom filter ...");

    bf->state_file = state_file;
    bf->load = load;
    bf->save = save;
    bf->free = free_memory;
    bf->add = add;
    bf->lookup = lookup;

    size_t bit64_length = bloom_filter_size >> 6;
    bit64 = safe_calloc(bit64_length, sizeof(uint64_t));
}