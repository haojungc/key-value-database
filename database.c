#include "database.h"
#include "bloomfilter.h"
#include "bptree.h"
#include "definition.h"
#include "sorting.h"
#include "utils.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

/* macros */
#define MAX_FILENAME_LENGTH 50
#define MAX_METADATA 200
#define MAX_KEY 2000000

/* static variables */
static FILE *fp = NULL;
static const char *dir_path = "storage";
static const char *meta_file_path = "storage/meta";
static const char *empty_str = "EMPTY";
static const char *newline = "\n";
static bloomfilter_t bf;
static char bf_file_path[MAX_FILENAME_LENGTH];
static bptree_t bptree;
static metadata_t metatable[MAX_METADATA];
static size_t meta_count = 0;
static int32_t loaded_file = -1;
static data_t *put_buf;
static size_t key_count = 0;
static uint64_t min_key = UINT64_MAX;
static uint64_t max_key = 0;
static bool first_line = true;

/* static function prototypes */
static void close();
static void set_output_filename(const char *filename);
static void put(const uint64_t key, char *value);
static void get(const uint64_t key);
static void scan(const uint64_t start_key, const uint64_t end_key);
static void load_metatable();
static void save_metatable();
/* Saves the first half of the buffer to the file and loads the file found from
 * the metatable. */
static void swap_files(metadata_t *metadata);
static void sort_put_buffer(const int32_t start, const int32_t end);
/* Flushes the PUT buffer by inserting the data into B+ tree. */
static void flush_put_buffer();

/* static functions */
static void close() {
    puts("closing database ...");

    bf.save(bf_file_path);
    bf.free();

    /* Flushes the buffer to B+ tree */
    flush_put_buffer();

    /* Saves B+ tree */
    if (!bptree.is_empty()) {
        char filepath[MAX_FILENAME_LENGTH + 1];
        size_t file_number = (loaded_file == -1) ? meta_count++ : loaded_file;
        metatable[file_number].file_number = file_number;
        snprintf(filepath, MAX_FILENAME_LENGTH, "%s/%lu", dir_path,
                 file_number);
        bptree.save(&metatable[file_number], filepath);
    }
    bptree.free_memory();

    save_metatable();

    for (int i = 0; i < MAX_KEY; i++) {
        free(put_buf[i].value);
    }
    free(put_buf);

    if (fp != NULL)
        fclose(fp);
}

static void set_output_filename(const char *filename) {
    fp = safe_fopen(filename, "wb");
}

static void put(const uint64_t key, char *value) {
    bf.add(key);

    /* Adds key-value to the buffer */
    put_buf[key_count].key = key;
    strncpy(put_buf[key_count].value, value, VALUE_LENGTH);
    key_count++;

    if (key_count < MAX_KEY) {
        return;
    }

    flush_put_buffer();
}

static void get(const uint64_t key) {
    int_fast8_t result = bf.lookup(key);
    /* Not found */
    if (result == -1) {
        /* Writes to output file */
        if (first_line) {
            first_line = false;
        } else {
            safe_fwrite(newline, sizeof(char), 1, fp);
        }
        safe_fwrite(empty_str, sizeof(char), strlen(empty_str), fp);
        return;
    }

    flush_put_buffer();

    /* Not in current B+ tree */
    if (key < min_key || key > max_key) {
        /* Looks up the metatable */
        bool found = false;
        for (int i = 0; i < meta_count; i++) {
            found =
                (key >= metatable[i].start_key && key <= metatable[i].end_key);
            if (found) {
                swap_files(&metatable[i]);
                min_key = metatable[i].start_key;
                max_key = metatable[i].end_key;
                break;
            }
        }
        if (!found) {
            if (first_line) {
                first_line = false;
            } else {
                safe_fwrite(newline, sizeof(char), 1, fp);
            }
            safe_fwrite(empty_str, sizeof(char), strlen(empty_str), fp);
            return;
        }
    }

    /* Writes the result to output file */
    if (first_line) {
        first_line = false;
    } else {
        safe_fwrite(newline, sizeof(char), 1, fp);
    }
    const char *value = bptree.search(key);
    if (value == NULL) {
        safe_fwrite(empty_str, sizeof(char), strlen(empty_str), fp);
    } else {
        safe_fwrite(value, sizeof(char), VALUE_LENGTH, fp);
    }
}

static void scan(const uint64_t start_key, const uint64_t end_key) {
    flush_put_buffer();

    char **ptrs = safe_malloc((end_key - start_key + 1) * sizeof(char *));
    for (uint64_t key = start_key; key <= end_key;) {
        /* Not in current B+ tree */
        if (key < min_key || key > max_key) {
            /* Looks up the metatable */
            bool found = false;
            for (int i = 0; i < meta_count; i++) {
                found = (key >= metatable[i].start_key &&
                         key <= metatable[i].end_key);
                if (found) {
                    swap_files(&metatable[i]);
                    min_key = metatable[i].start_key;
                    max_key = metatable[i].end_key;

                    /* Sets ptrs */
                    uint64_t _end_key = MIN(end_key, max_key);
                    size_t size = _end_key - key + 1;
                    memset(ptrs, 0, size * sizeof(char *));
                    bptree.scan(ptrs, key, _end_key);

                    /* Writes ptrs to output file */
                    for (int j = 0; j < size; j++) {
                        if (first_line) {
                            first_line = false;
                        } else {
                            safe_fwrite(newline, sizeof(char), 1, fp);
                        }
                        if (ptrs[j] == NULL) {
                            safe_fwrite(empty_str, sizeof(char),
                                        strlen(empty_str), fp);
                        } else {
                            safe_fwrite(ptrs[j], sizeof(char), VALUE_LENGTH,
                                        fp);
                        }
                    }
                    key = _end_key + 1;
                    break;
                }
            }
            if (!found) {
                if (first_line) {
                    first_line = false;
                } else {
                    safe_fwrite(newline, sizeof(char), 1, fp);
                }
                safe_fwrite(empty_str, sizeof(char), strlen(empty_str), fp);
                key++;
                continue;
            }
        } else {
            /* Sets ptrs */
            uint64_t _end_key = MIN(end_key, max_key);
            size_t size = _end_key - key + 1;
            memset(ptrs, 0, size * sizeof(char *));
            bptree.scan(ptrs, key, _end_key);

            /* Writes ptrs to output file */
            for (int i = 0; i < size; i++) {
                if (first_line) {
                    first_line = false;
                } else {
                    safe_fwrite(newline, sizeof(char), 1, fp);
                }
                if (ptrs[i] == NULL) {
                    safe_fwrite(empty_str, sizeof(char), strlen(empty_str), fp);
                } else {
                    safe_fwrite(ptrs[i], sizeof(char), VALUE_LENGTH, fp);
                }
            }
            key = _end_key + 1;
        }
    }
    free(ptrs);
}

static void load_metatable() {
    puts("loading metatable ...");
    FILE *file = safe_fopen(meta_file_path, "rb");
    metadata_t *ptr = metatable;
    while (fread(ptr, sizeof(metadata_t), 1, file) == 1) {
        meta_count++;
        ptr++;
    }
    fclose(file);

    DEBUG(for (int i = 0; i < meta_count; i++) {
        printf("file_number: %lu, start: %lu, end: %lu, total_keys: %lu\n",
               metatable[i].file_number, metatable[i].start_key,
               metatable[i].end_key, metatable[i].total_keys);
    })
}

static void save_metatable() {
    puts("saving metatable ...");
    FILE *file = safe_fopen(meta_file_path, "wb");
    safe_fwrite(metatable, sizeof(metadata_t), meta_count, file);
    fclose(file);
}

static void swap_files(metadata_t *metadata) {
    printf("swapping to file %lu ...\n", metadata->file_number);

    static char filepath[MAX_FILENAME_LENGTH + 1];

    /* Saves B+ tree */
    if (!bptree.is_empty()) {
        size_t file_number = (loaded_file == -1) ? meta_count++ : loaded_file;
        metatable[file_number].file_number = file_number;
        snprintf(filepath, MAX_FILENAME_LENGTH, "%s/%lu", dir_path,
                 file_number);
        bptree.save(&metatable[file_number], filepath);
    }

    /* Loads B+ tree */
    snprintf(filepath, MAX_FILENAME_LENGTH, "%s/%lu", dir_path,
             metadata->file_number);
    bptree.load(metadata, filepath);

    loaded_file = metadata->file_number;
}

static void sort_put_buffer(const int32_t start, const int32_t end) {
    if (end - start <= 1) {
        return;
    }
    /* stable sort */
    mergesort(put_buf, start, end);
}

static void flush_put_buffer() {
    DEBUG(printf("keys in buffer: %lu\n", key_count);)

    if (key_count == 0) {
        return;
    }

    sort_put_buffer(0, key_count - 1);

    uint64_t key;
    char *value;
    for (int i = 0; i < key_count; i++) {
        key = put_buf[i].key;
        value = put_buf[i].value;

        /* Flushes the B+ tree */
        if (bptree.is_full()) {
            /* Splits B+ tree into two parts and saves one of them depending on
             * the current key */
            char file[MAX_FILENAME_LENGTH + 1];
            size_t file_number =
                (loaded_file == -1) ? meta_count++ : loaded_file;
            metatable[file_number].file_number = file_number;
            snprintf(file, MAX_FILENAME_LENGTH, "%s/%lu", dir_path,
                     file_number);
            bptree.split_and_save_one(&metatable[file_number], file, key);
            loaded_file = -1;
            min_key = bptree.get_min_key();
            max_key = bptree.get_max_key();
            DEBUG(printf("min_key: %lu, max_key: %lu\n", min_key, max_key);)
        }

        if (key < min_key || key > max_key) {
            /* Looks up the metatable */
            bool found = false;
            uint64_t min_diff = (key < min_key) ? min_key - key : key - max_key;
            int32_t min_diff_idx = -1;
            for (int i = 0; i < meta_count; i++) {
                bool loaded = (metatable[i].file_number == loaded_file);
                found = (!loaded && key >= metatable[i].start_key &&
                         key <= metatable[i].end_key);
                if (found) {
                    swap_files(&metatable[i]);
                    min_key = metatable[i].start_key;
                    max_key = metatable[i].end_key;
                    break;
                }
                /* Finds the file whose start key is larger than and nearest
                 * to the key */
                if (key < metatable[i].start_key) {
                    uint64_t diff = metatable[i].start_key - key;
                    if (diff < min_diff) {
                        min_diff = diff;
                        min_diff_idx = i;
                    }
                } else if (key > metatable[i].end_key) {
                    /* Finds the file whose end key is less than and nearest to
                     * the key */
                    uint64_t diff = key - metatable[i].end_key;
                    if (diff < min_diff) {
                        min_diff = diff;
                        min_diff_idx = i;
                    }
                }
            }
            if (!found) {
                /* Swaps to the file whose start key or end key is nearest to
                 * the key */
                bool need_to_swap =
                    (min_diff_idx != -1 &&
                     metatable[min_diff_idx].file_number != loaded_file);
                if (need_to_swap) {
                    swap_files(&metatable[min_diff_idx]);
                    min_key = MIN(key, metatable[min_diff_idx].start_key);
                    max_key = MAX(key, metatable[min_diff_idx].end_key);
                } else {
                    /* Already loaded the file whose start key or end key is
                     * nearest to the key */
                    min_key = MIN(key, min_key);
                    max_key = MAX(key, max_key);
                }
            }
        }

        bptree.insert(key, value);
    }
    key_count = 0;
}

/* extern functions */
void init_database(database_t *db) {
    puts("initializing database ...");

    db->close = close;
    db->set_output_filename = set_output_filename;
    db->put = put;
    db->get = get;
    db->scan = scan;

    /* Initializes the bloom filter and loads the previous bloom filter if
     * available. */
    init_bloomfilter(&bf);

    sprintf(bf_file_path, "%s/%s", dir_path, bf.state_file);
    safe_mkdir(dir_path, ACCESSPERMS);

    if (file_exists(bf_file_path) == 0)
        bf.load(bf_file_path);

    /* Loads the previous metatable if available */
    if (file_exists(meta_file_path) == 0)
        load_metatable();

    /* Initializes B+ tree */
    init_bptree(&bptree);

    put_buf = safe_malloc(MAX_KEY * sizeof(data_t));
    for (int i = 0; i < MAX_KEY; i++) {
        put_buf[i].value = safe_malloc((VALUE_LENGTH + 1) * sizeof(char));
    }
}