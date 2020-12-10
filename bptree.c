#include "bptree.h"
#include "utils.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define ORDER 5
#define MAX_KEY (ORDER - 1)
#define MAX_FILEPATH 50

static node_t *head = NULL;
static char filepath[MAX_FILEPATH];

/* static function prototypes */
static void load(const char *_filepath);
static void save();
static void free_memory();
static void insert(const uint64_t key, char *value);
static char *search(const uint64_t key);
// static void check();
// static void show();

/* Frees the memory allocated for node. */
static void free_node(node_t *node);
/* Frees the memory allocated for tree which the root is node. */
static void free_tree(node_t *node);
/* Gets the index where the key belongs to from the node. */
static int_fast8_t get_key_idx(const node_t *node, const uint64_t key);
/* Searches down from the root node and finds the leaf node where the key
 * belongs to. */
static node_t *find_leaf(node_t *root, const uint64_t key);
/* Creates and initializes a leaf node. */
static node_t *create_leaf();
/* Creates and initializes an internal node. */
static node_t *create_node();
/* Splits the overflowed leaf node into two parts and returns the pointer to the
 * new leaf node. */
static node_t *split_leaf(node_t *leaf, const uint64_t keys[], void *ptrs[]);
/* Splits the overflowed internal node into two parts and returns the pointer to
 * the new internal node. */
static node_t *split_node(node_t *node, const uint64_t keys[], void *ptrs[]);
/* Inserts a key and a value into leaf node. */
static void insert_into_leaf(node_t *leaf, const uint64_t key, char *value);
/* Inserts a key into internal node. */
static void insert_into_node(node_t *node, node_t *child, const uint64_t key);

/* static functions */
static void load(const char *_filepath) {}

static void save() {}

static void free_memory() {
    if (head != NULL) {
        free_tree(head);
        head = NULL;
    }
}

static void insert(const uint64_t key, char *value) {
    if (head == NULL) {
        node_t *leaf = create_leaf();
        head = leaf;
        leaf->keys[0] = key;
        leaf->ptrs[0] = value;
        leaf->key_count = 1;
        return;
    }

    node_t *node = find_leaf(head, key);
    insert_into_leaf(node, key, value);
}

static char *search(const uint64_t key) {
    node_t *node = find_leaf(head, key);
    for (int i = 0; i < node->key_count; i++) {
        if (node->keys[i] == key) {
            return (char *)node->ptrs[i];
        }
    }
    return NULL;
}

// static void check() {
//     if (head == NULL) {
//         return;
//     }

//     /* Traverses down until leaf node is reached */
//     node_t *node = head;
//     int_fast8_t level = 1;
//     while (node->is_leaf == false) {
//         node = node->ptrs[0];
//         level++;
//     }
//     printf("total levels: %d\n", level);

//     /* Warning: error occurs if the first key in B+ tree is 0 */
//     uint64_t prev_key = 0;
//     uint_fast32_t total_keys = 0;
//     while (node != NULL) {
//         for (int i = 0; i < node->key_count; i++) {
//             if (node->keys[i] <= prev_key) {
//                 fprintf(stderr, "keys are not arranged in increasing
//                 order\n"); printf("prev_key: %lu\n", prev_key); for (int i =
//                 0; i < node->key_count; i++) {
//                     printf("%lu ", node->keys[i]);
//                 }
//                 printf("\n");
//                 exit(EXIT_FAILURE);
//             }
//             total_keys++;
//             prev_key = node->keys[i];
//         }
//         node = node->next;
//     }
//     printf("total keys: %lu\n", total_keys);
//     printf("\nSuccessful!\n");
// }

// static void show() {
//     if (head == NULL) {
//         return;
//     }

//     /* Traverses down until leaf node is reached */
//     node_t *node = head;
//     int_fast8_t level = 1;
//     while (node->is_leaf == false) {
//         node = node->ptrs[0];
//         level++;
//     }
//     printf("total levels: %d\n", level);

//     /* Prints out all the leaf nodes */
//     uint_fast32_t total_nodes = 0;
//     uint_fast32_t total_keys = 0;
//     while (node != NULL) {
//         printf("node %lu: ", total_nodes++);
//         for (int i = 0; i < node->key_count; i++) {
//             printf("%lu ", node->keys[i]);
//             fflush(stdout);
//             total_keys++;
//         }
//         printf("\n");
//         node = node->next;
//     }
//     printf("total keys: %lu\n", total_keys);
// }

static void free_node(node_t *node) {
    free(node->keys);
    free(node->ptrs);
    free(node);
}

static void free_tree(node_t *node) {
    if (node->is_leaf) {
        free_node(node);
        return;
    }
    /* Postorder Traversal */
    for (int i = 0; i < node->key_count + 1; i++) {
        free_tree(node->ptrs[i]);
    }
    free_node(node);
}

static int_fast8_t get_key_idx(const node_t *node, const uint64_t key) {
    int_fast8_t idx;
    for (idx = 0; idx < node->key_count; idx++) {
        if (key <= node->keys[idx]) {
            return idx;
        }
    }
    return idx;
}

static node_t *find_leaf(node_t *root, const uint64_t key) {
    /* Traverses down until leaf node is reached */
    node_t *node = root;
    while (node->is_leaf == false) {
        int_fast8_t i;
        for (i = 0; i < node->key_count; i++) {
            if (key < node->keys[i]) {
                break;
            }
        }
        node = node->ptrs[i];
    }
    return node;
}

static node_t *create_leaf() {
    node_t *node = safe_calloc(1, sizeof(node_t));
    node->is_leaf = true;
    node->keys = safe_malloc(MAX_KEY * sizeof(uint64_t));
    node->ptrs = safe_malloc(MAX_KEY * sizeof(char *));
    return (node_t *)node;
}

static node_t *create_node() {
    node_t *node = safe_calloc(1, sizeof(node_t));
    node->is_leaf = false;
    node->keys = safe_malloc(MAX_KEY * sizeof(uint64_t));
    node->ptrs = safe_calloc(ORDER, sizeof(node_t *));
    return (node_t *)node;
}

static node_t *split_leaf(node_t *leaf, const uint64_t keys[], void *ptrs[]) {
    node_t *new_leaf = create_leaf();

    /* Moves keys and values in the second half of the current leaf to the new
     * leaf (including the median key and its corresponding value) */
    int_fast8_t total_keys = leaf->key_count;
    int_fast8_t median_idx = total_keys >> 1;
    for (int i = 0; i < median_idx; i++) {
        leaf->keys[i] = keys[i];
        leaf->ptrs[i] = ptrs[i];
    }
    for (int i = median_idx; i < total_keys; i++) {
        new_leaf->keys[i - median_idx] = keys[i];
        new_leaf->ptrs[i - median_idx] = ptrs[i];
    }

    /* Updates information about the current leaf and the new leaf */
    leaf->key_count = median_idx;
    new_leaf->key_count = total_keys - leaf->key_count;
    new_leaf->next = leaf->next;
    leaf->next = new_leaf;
    return new_leaf;
}

static node_t *split_node(node_t *node, const uint64_t keys[], void *ptrs[]) {
    node_t *new_node = create_node();
    int_fast8_t total_keys = node->key_count;
    int_fast8_t median_idx = total_keys >> 1;

    /* Updates the current node */
    for (int i = 0; i < median_idx; i++) {
        node->keys[i] = keys[i];
        node->ptrs[i] = ptrs[i];
    }
    node->ptrs[median_idx] = ptrs[median_idx];
    node->key_count = median_idx;

    /* Moves keys and values in the second half of keys and ptrs to the new
     * node (excluding the median key) */
    for (int i = median_idx + 1; i < total_keys; i++) {
        new_node->keys[i - (median_idx + 1)] = keys[i];
        new_node->ptrs[i - (median_idx + 1)] = ptrs[i];
        /* Updates children's parent */
        ((node_t *)new_node->ptrs[i - (median_idx + 1)])->parent = new_node;
    }

    /* Subtracts by one because the median key is moved upwards */
    int_fast8_t new_key_count = total_keys - node->key_count - 1;
    new_node->key_count = new_key_count;
    new_node->ptrs[new_key_count] = ptrs[total_keys];
    /* Updates children's parent */
    ((node_t *)new_node->ptrs[new_key_count])->parent = new_node;
    return new_node;
}

static void insert_into_leaf(node_t *leaf, const uint64_t key, char *value) {
    static uint64_t keys[MAX_KEY + 1];
    static void *ptrs[MAX_KEY + 1];
    int_fast8_t inserted_idx = get_key_idx(leaf, key);

    /* Overwrites the existing value */
    if (inserted_idx < leaf->key_count && key == leaf->keys[inserted_idx]) {
        leaf->ptrs[inserted_idx] = value;
        return;
    }

    /* Sets the buffer values */
    for (int i = 0; i < inserted_idx; i++) {
        keys[i] = leaf->keys[i];
        ptrs[i] = leaf->ptrs[i];
    }
    keys[inserted_idx] = key;
    ptrs[inserted_idx] = value;
    for (int i = inserted_idx; i < leaf->key_count; i++) {
        keys[i + 1] = leaf->keys[i];
        ptrs[i + 1] = leaf->ptrs[i];
    }
    leaf->key_count++;

    bool overflowed = (leaf->key_count > MAX_KEY);
    if (overflowed) {
        /* Creates a leaf node and an internal node. The leaf node is used for
         * storing keys and values in the second half of current leaf, and the
         * internal is used for storing the median key found in the current
         * leaf. */
        node_t *new_leaf = split_leaf(leaf, keys, ptrs);

        // printf("key %lu is moved upwards (leaf to node)\n",
        // new_leaf->keys[0]);
        if (leaf->parent == NULL) {
            // puts("leaf node's parent is NULL");
            node_t *new_parent = create_node();
            new_parent->keys[0] = new_leaf->keys[0];
            new_parent->ptrs[0] = leaf;
            new_parent->ptrs[1] = new_leaf;
            new_parent->key_count = 1;
            head = new_parent;
            leaf->parent = new_parent;
            new_leaf->parent = new_parent;
        } else {
            new_leaf->parent = leaf->parent;
            insert_into_node(new_leaf->parent, new_leaf, new_leaf->keys[0]);
        }
    } else {
        for (int i = 0; i < leaf->key_count; i++) {
            leaf->keys[i] = keys[i];
            leaf->ptrs[i] = ptrs[i];
        }
    }
}

static void insert_into_node(node_t *node, node_t *child, const uint64_t key) {
    static uint64_t keys[MAX_KEY + 1];
    static void *ptrs[ORDER + 1];
    int_fast8_t inserted_idx = get_key_idx(node, key);

    /* Sets the buffer values */
    for (int i = 0; i < inserted_idx; i++) {
        keys[i] = node->keys[i];
        ptrs[i] = node->ptrs[i];
    }
    keys[inserted_idx] = key;
    ptrs[inserted_idx] = node->ptrs[inserted_idx];
    ptrs[inserted_idx + 1] = child;
    for (int i = inserted_idx; i < node->key_count; i++) {
        keys[i + 1] = node->keys[i];
        ptrs[i + 2] = node->ptrs[i + 1];
    }
    node->key_count++;

    bool overflowed = (node->key_count > MAX_KEY);
    if (overflowed) {
        // puts("internal node overflowed");

        /* If the parent node is full after inserting the key, create two
         * internal nodes, one for storing keys in the second half (excluding
         * the median key) of current node, the other for storing the median key
         * (parent). Then move the median key upwards to the second node. Repeat
         * the process until the new internal node is not full. */
        /* current key_count: MAX_KEY + 1 */
        int_fast8_t median_idx = node->key_count >> 1;
        node_t *new_neighbor = split_node(node, keys, ptrs);

        /* Moves the median key upwards */
        // printf("key %lu is moved upwards (node to node)\n",
        // keys[median_idx]);
        if (node->parent == NULL) {
            // puts("internal node's parent is NULL");
            node_t *new_parent = create_node();
            new_parent->keys[0] = keys[median_idx];
            new_parent->ptrs[0] = node;
            new_parent->ptrs[1] = new_neighbor;
            new_parent->key_count = 1;
            head = new_parent;
            node->parent = new_parent;
            new_neighbor->parent = new_parent;
        } else {
            new_neighbor->parent = node->parent;
            insert_into_node(new_neighbor->parent, new_neighbor,
                             keys[median_idx]);
        }
    } else {
        int_fast8_t last_idx = node->key_count;
        for (int i = 0; i < last_idx; i++) {
            node->keys[i] = keys[i];
            node->ptrs[i] = ptrs[i];
        }
        node->ptrs[last_idx] = ptrs[last_idx];
    }
}

/* extern functions */
void init_bptree(bptree_t *bptree) {
    bptree->load = load;
    bptree->save = save;
    bptree->free_memory = free_memory;
    bptree->insert = insert;
    bptree->search = search;
    // bptree->check = check;
    // bptree->show = show;
}

#undef ORDER
#undef MAX_KEY