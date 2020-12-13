#ifndef BPTREE_H
#define BPTREE_H
#include "definition.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct node {
    bool is_leaf;
    uint64_t *keys;
    /* is_leaf == true: ptrs[i] points to the ith data;
     * is_leaf == false (internal node): ptrs[i] points to the ith child node */
    void **ptrs;
    int_fast8_t key_count;
    struct node *parent;
    /* is_leaf == true: next points to the next leaf;
     * is_leaf == false (internal node): next is a null pointer */
    struct node *next;
} node_t;

typedef struct bptree {
    void (*load)(const char *filepath, const uint64_t total_keys);
    void (*save)(metadata_t *metadata, const char *filepath);
    void (*split_and_save_one)(metadata_t *metadata, const char *filepath,
                               const uint64_t key);
    void (*free_memory)();
    void (*insert)(const uint64_t key, char *value);
    const char *(*search)(const uint64_t key);
    void (*scan)(char *ptrs[], const uint64_t start_key,
                 const uint64_t end_key);
    int_fast8_t (*is_empty)();
    int_fast8_t (*is_full)();
    uint64_t (*get_min_key)();
    uint64_t (*get_max_key)();
    // void (*check)();
    // void (*show)();
} bptree_t;

void init_bptree(bptree_t *bptree);

#endif