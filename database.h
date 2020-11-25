#ifndef DATABASE_H
#define DATABASE_H
#include <stdint.h>

/* Initializes the database and sets the output file name. */
void init_database(const char *filename);
void close_database();
void put(const uint64_t key, const char *value);
void get(const uint64_t key);
void scan(const uint64_t key1, const uint64_t key2);

#endif