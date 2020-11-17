#ifndef DATABASE_H
#define DATABASE_H
#include <stdint.h>

void set_output_file(const char *filename);
void close_output_file();
void put(const uint64_t key, const char *value);
const char *get(const uint64_t key);
void scan(const uint64_t key1, const uint64_t key2);

#endif