#ifndef DEFINITION_H
#define DEFINITION_H
#include <stddef.h>
#include <stdint.h>

#define MAX_PATH 50
#define VALUE_LENGTH 128

typedef struct {
    size_t file_number;
    uint64_t start_key;
    uint64_t end_key;
    size_t total_keys;
} metadata_t;

typedef struct data {
    uint64_t key;
    char *value;
} data_t;

#endif