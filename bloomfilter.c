#include "bloomfilter.h"
#include "utils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

const char *bf_filename = "bf.state";
static uint64_t *bit64;
static size_t bloom_filter_size;

void init_bloom_filter(size_t size) {
    printf("initializing bloom filter...");
    fflush(stdout);
    bloom_filter_size = size;
    size_t len = size >> 6;
    bit64 = safe_calloc(len, sizeof(uint64_t));
    puts("done");
}

void load_bloom_filter(const char *file_path) {
    printf("loading bloom filter...");
    fflush(stdout);
    FILE *fp = safe_fopen(file_path, "rb");
    safe_fread(bit64, sizeof(uint64_t), bloom_filter_size >> 6, fp);
    fclose(fp);
    puts("done");
}

void save_bloom_filter(const char *file_path) {
    printf("saving bloom filter...");
    fflush(stdout);
    FILE *fp = safe_fopen(file_path, "wb");
    safe_fwrite(bit64, sizeof(uint64_t), bloom_filter_size >> 6, fp);
    fclose(fp);
    puts("done");
}

void free_bloom_filter() { free(bit64); }

void set_bloom_filter(const uint64_t key) {}

int_fast8_t lookup_bloom_filter(const uint64_t key) {}