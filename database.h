#ifndef DATABASE_H
#define DATABASE_H
#include <stdint.h>

void init_database();
void close_database();
void set_output_filename(const char *filename);
void put(const uint64_t key, const char *value);
void get(const uint64_t key);
void scan(const uint64_t key1, const uint64_t key2);

#endif