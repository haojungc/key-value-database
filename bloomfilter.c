#include "bloomfilter.h"
#include "utils.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

const char *bf_filename = "bf.state";
static uint64_t *bit64;
static size_t bloom_filter_size;

void init_bloom_filter(size_t size) {
    puts("initializing bloom filter ...");
    bloom_filter_size = size;
    size_t len = size >> 6;
    bit64 = safe_calloc(len, sizeof(uint64_t));
}

void load_bloom_filter(const char *file_path) {
    puts("loading bloom filter ...");
    FILE *fp = safe_fopen(file_path, "rb");
    safe_fread(bit64, sizeof(uint64_t), bloom_filter_size >> 6, fp);
    fclose(fp);
}

void save_bloom_filter(const char *file_path) {
    puts("saving bloom filter ...");
    /* TODO:
     * reduce the overhead: http://www.cplusplus.com/reference/cstdio/rewind/ */
    FILE *fp = safe_fopen(file_path, "wb");
    safe_fwrite(bit64, sizeof(uint64_t), bloom_filter_size >> 6, fp);
    fclose(fp);
}

void free_bloom_filter() { free(bit64); }

void set_bloom_filter(const uint64_t key) {}

int_fast8_t lookup_bloom_filter(const uint64_t key) {}