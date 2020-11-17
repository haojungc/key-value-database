#include "database.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

static FILE *fp;

void set_output_file(const char *filename) { fp = safe_fopen(filename, "w"); }
void close_output_file() { fclose(fp); }
void put(const uint64_t key, const char *value) {}
const char *get(const uint64_t key) {}
void scan(const uint64_t key1, const uint64_t key2) {}