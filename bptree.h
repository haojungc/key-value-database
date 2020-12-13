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
    /* Loads records from file. */
    void (*load)(const char *filepath, const uint64_t total_keys);
    /* Saves the B+ tree to file and updates metadata. */
    void (*save)(metadata_t *metadata, const char *filepath);
    /* Splits the B+ tree into two parts and saves one of the two to file,
     * according to key. */
    void (*split_and_save_one)(metadata_t *metadata, const char *filepath,
                               const uint64_t key);
    /* Frees the memory allocated for the B+ tree. */
    void (*free_memory)();
    /* Inserts a record into the B+ tree. */
    void (*insert)(const uint64_t key, char *value);
    /* Searches key in the B+ tree. */
    const char *(*search)(const uint64_t key);
    /* Scans from start key to end key and assigns the address of the value (if
     * found) to the pointer array. Note that the pointer array is initialized
     * to NULL before passing it to this function. */
    void (*scan)(char *ptrs[], const uint64_t start_key,
                 const uint64_t end_key);
    /* Returns a non zero value if the tree is empty, and 0 otherwise. */
    int_fast8_t (*is_empty)();
    /* Returns a non zero value if the tree is full, and 0 otherwise. */
    int_fast8_t (*is_full)();
    /* Returns the minimum key in the tree. */
    uint64_t (*get_min_key)();
    /* Returns the maximum key in the tree. */
    uint64_t (*get_max_key)();
    // void (*check)();
    // void (*show)();
} bptree_t;

void init_bptree(bptree_t *bptree);

#endif