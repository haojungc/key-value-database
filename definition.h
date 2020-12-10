#ifndef DEFINITION_H
#define DEFINITION_H
#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t file_number;
    uint64_t start_key;
    uint64_t end_key;
    size_t total_keys;
} metadata_t;

#endif