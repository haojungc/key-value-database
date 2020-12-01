#include "database.h"
#include "bloomfilter.h"
#include "skiplist.h"
#include "utils.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

/* static variables */
static FILE *fp = NULL;
static const char *dir_path = "storage";
#define MAX_FILENAME_LENGTH 50
static char bf_file_path[MAX_FILENAME_LENGTH];
#undef MAX_FILENAME_LENGTH

/* extern functions */
void init_database() {
    sprintf(bf_file_path, "%s/%s", dir_path, bf_filename);
    safe_mkdir(dir_path, ACCESSPERMS);
    /* Initializes the bloom filter.
     * Loads the previous bloom filter if available */
    init_bloom_filter();
    if (file_exists(bf_file_path) == 0)
        load_bloom_filter(bf_file_path);
}

void close_database() {
    if (fp != NULL)
        fclose(fp);
    save_bloom_filter(bf_file_path);
    free_bloom_filter();
}

void set_output_filename(const char *filename) {
    fp = safe_fopen(filename, "w");
}

void put(const uint64_t key, const char *value) {
    set_bloom_filter(key);
    /* skip list */
}

void get(const uint64_t key) {
    int_fast8_t result = lookup_bloom_filter(key);
    /* Not found */
    if (result == -1) {
        /* Write to output file */
        return;
    }
    /* Search key in database */
    /* skip list */
}

void scan(const uint64_t key1, const uint64_t key2) {
    /* parallelizable? */
    for (uint64_t i = key1; i <= key2; i++)
        get(i);
}