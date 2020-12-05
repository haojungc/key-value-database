#include "database.h"
#include "bloomfilter.h"
#include "utils.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

/* macros */
#define MIN(x, y) ((x) < (y) ? (x) : (y))

#define MAX_FILENAME_LENGTH 50
#define MAX_METADATA 100
#define VALUE_LENGTH 128
#define MAX_KEY 2000000
#define MAX_KEY_PER_FILE (MAX_KEY / 2)
#define A_START 0
#define A_END (MAX_KEY / 2 - 1)
#define B_START (MAX_KEY / 2)
#define B_END (MAX_KEY - 1)

typedef struct {
    size_t file_number;
    uint64_t start_key;
    uint64_t end_key;
    size_t total_keys;
} metadata_t;

typedef struct {
    uint64_t key;
    char *value;
} data_t;

/* static variables */
static FILE *fp = NULL;
static const char *dir_path = "storage";
static const char *meta_file_path = "storage/meta";
static char bf_file_path[MAX_FILENAME_LENGTH];
static metadata_t metatable[MAX_METADATA];
static size_t meta_count = 0;
static int32_t loaded_file = -1;
static data_t buf[MAX_KEY];
static size_t key_count = 0;

/* static function prototypes */
static void load_metatable();
static void save_metatable();
/* Saves the buffer to a file (from start to end, inclusively). If there exists
 * a file that has been loaded before saving the buffer, the filename is set to
 * the name of the loaded file. Otherwise, the filename is the current number of
 * existing files (filename starts from "0").
 */
static void save_buffer_to_file(const int32_t start, const int32_t end);
/* Saves the first half of the buffer to the file and loads the file found from
 * the metatable. */
static void swap_files(const metadata_t *metadata);
/* Appends a buffer entry to the buffer. */
static void append_to_buffer(const uint64_t key, const char *value);
/* Overwrites a buffer entry. */
static void overwrite_buffer_entry(const uint64_t key, const char *value,
                                   const int idx);
/* Inserts a buffer entry into the buffer after an given index. */
static void insert_after(const uint64_t key, const char *value, const int idx);

/* static functions */
static void load_metatable() {
    puts("loading metatable ...");
    FILE *file = safe_fopen(meta_file_path, "rb");
    metadata_t *ptr = metatable;
    while (fread(ptr, sizeof(metadata_t), 1, file) == 1) {
        meta_count++;
        ptr++;
    }
    fclose(file);

    /* Test output */
    for (int i = 0; i < meta_count; i++) {
        printf("file_number: %lu, start: %lu, end: %lu, total_keys: %lu\n",
               metatable[i].file_number, metatable[i].start_key,
               metatable[i].end_key, metatable[i].total_keys);
    }
}

static void save_metatable() {
    puts("saving metatable ...");
    FILE *file = safe_fopen(meta_file_path, "wb");
    safe_fwrite(metatable, sizeof(metadata_t), meta_count, file);
    fclose(file);
}

static void save_buffer_to_file(const int32_t start, const int32_t end) {
    if (end < start)
        return;
    printf("saving buffer from %d to %d ...\n", start, end);

    /* Updates metatable */
    size_t file_number = (loaded_file == -1) ? meta_count++ : loaded_file;
    metatable[file_number].file_number = file_number;
    metatable[file_number].start_key = buf[start].key;
    metatable[file_number].end_key = buf[end].key;
    metatable[file_number].total_keys = end - start + 1;

    /* Saves the buffer to the file */
    FILE *file;
    static char filepath[MAX_FILENAME_LENGTH + 1];
    snprintf(filepath, MAX_FILENAME_LENGTH, "%s/%lu", dir_path, file_number);
    file = safe_fopen(filepath, "wb");
    for (int i = start; i <= end; i++) {
        /* TODO: change the way of storing data */
        safe_fwrite(&buf[i].key, sizeof(uint64_t), 1, file);
        safe_fwrite(buf[i].value, sizeof(char), VALUE_LENGTH, file);
    }
    fclose(file);
    loaded_file = -1; /* No file is loaded */
}

static void swap_files(const metadata_t *metadata) {
    printf("swapping to file %lu ...\n", metadata->file_number);

    save_buffer_to_file(A_START, MIN(key_count, MAX_KEY_PER_FILE) - 1);

    /* Moves the remaining keys to the beginning of the buffer */
    /* TODO: find a way to enhance efficiency */
    size_t remaining_keys = 0;
    for (int i = B_START; i < key_count; i++)
        buf[remaining_keys++] = buf[i];
    key_count = remaining_keys;

    static char filepath[MAX_FILENAME_LENGTH + 1];
    snprintf(filepath, MAX_FILENAME_LENGTH, "%s/%lu", dir_path,
             metadata->file_number);
    FILE *file = safe_fopen(filepath, "rb");
    loaded_file = metadata->file_number;

    /* Loads the data from the file */
    size_t total_keys = metadata->total_keys;
    for (int i = 0; i < total_keys; i++) {
        /* TODO: change the way of reading data */
        safe_fread(&buf[key_count + i].key, sizeof(uint64_t), 1, file);
        safe_fread(buf[key_count + i].value, sizeof(char), VALUE_LENGTH, file);
        // printf("key: %lu, value %s\n", buf[key_count + i].key,
        //        buf[key_count + i].value);
    }
    key_count += metadata->total_keys;

    fclose(file);
}

static void append_to_buffer(const uint64_t key, const char *value) {
    overwrite_buffer_entry(key, value, key_count);
    key_count++;
}

static void overwrite_buffer_entry(const uint64_t key, const char *value,
                                   const int idx) {
    buf[idx].key = key;
    strncpy(buf[idx].value, value, VALUE_LENGTH);
}

static void insert_after(const uint64_t key, const char *value, const int idx) {
    char *tmp = buf[key_count].value;
    /* TODO: find a way to enhance efficiency */
    for (int i = key_count - 1; i > idx; i--) {
        buf[i + 1].key = buf[i].key;
        buf[i + 1].value = buf[i].value;
    }
    buf[idx + 1].value = tmp;
    overwrite_buffer_entry(key, value, idx + 1);
    key_count++;
}

/* extern functions */
void init_database() {
    puts("initializing database ...");

    sprintf(bf_file_path, "%s/%s", dir_path, bf_filename);
    safe_mkdir(dir_path, ACCESSPERMS);

    /* Initializes the bloom filter and loads the previous bloom filter if
     * available. */
    init_bloom_filter();
    if (file_exists(bf_file_path) == 0)
        load_bloom_filter(bf_file_path);

    /* Loads the previous metatable if available */
    if (file_exists(meta_file_path) == 0)
        load_metatable();

    /* Allocates memory for the buffer */
    for (int i = 0; i < MAX_KEY; i++)
        buf[i].value = safe_malloc((VALUE_LENGTH + 1) * sizeof(char));
}

void close_database() {
    puts("closing database ...");

    save_buffer_to_file(A_START, MIN(key_count, MAX_KEY_PER_FILE) - 1);
    if (key_count > MAX_KEY_PER_FILE) {
        save_buffer_to_file(B_START, key_count - 1);
    }
    save_metatable();
    save_bloom_filter(bf_file_path);
    free_bloom_filter();
    for (int i = 0; i < MAX_KEY; i++)
        free(buf[i].value);
    if (fp != NULL)
        fclose(fp);
}

void set_output_filename(const char *filename) {
    fp = safe_fopen(filename, "w");
}

void put(const uint64_t key, const char *value) {
    set_bloom_filter(key);

    /* PUT data */
    if (key_count == 0) {
        /* Looks up metatable */
        bool found = false;
        for (int i = 0; i < meta_count; i++) {
            if (metatable[i].total_keys < MAX_KEY_PER_FILE &&
                key >= metatable[i].start_key && key <= metatable[i].end_key) {
                swap_files(&metatable[i]);
                found = true;
                break;
            }
        }
        if (found) {
            put(key, value);
        } else {
            append_to_buffer(key, value);
        }
    } else {
        bool belong_to_buf = key >= buf[0].key && key <= buf[key_count - 1].key;
        if (belong_to_buf) {
            /* TODO: too slow! implement B+ tree? */
            /* Binary search */
            uint32_t start = 0, end = key_count;
            uint32_t idx;
            while (true) {
                idx = (start + end) >> 1;
                if (key == buf[idx].key) {
                    overwrite_buffer_entry(key, value, idx);
                    break;
                } else if (key < buf[idx].key) {
                    if (key > buf[idx - 1].key) {
                        insert_after(key, value, idx - 1);
                        break;
                    }
                    end = idx;
                } else {
                    if (key < buf[idx + 1].key) {
                        insert_after(key, value, idx);
                        break;
                    }
                    start = idx;
                }
            }
        } else {
            /* Looks up metatable */
            bool found = false;
            for (int i = 0; i < meta_count; i++) {
                if (key >= metatable[i].start_key &&
                    key <= metatable[i].end_key) {
                    swap_files(&metatable[i]);
                    found = true;
                    break;
                }
            }
            if (found) {
                put(key, value);
            } else if (key < buf[0].key) {
                /* Inserts to the beginning of the buffer */
                insert_after(key, value, -1);
            } else if (key > buf[key_count - 1].key) {
                /* Appends the end of the buffer */
                append_to_buffer(key, value);
            }
        }
    }

    /* Writes the second half (part B) of the buffer to a file */
    if (key_count == MAX_KEY) {
        save_buffer_to_file(B_START, B_END);
        key_count -= MAX_KEY_PER_FILE;
    }
}

void get(const uint64_t key) {
    int_fast8_t result = lookup_bloom_filter(key);
    /* Not found */
    if (result == -1) {
        /* Write to output file */
        return;
    }
    /* Search key in database */
}

void scan(const uint64_t key1, const uint64_t key2) {
    /* parallelizable? */
    for (uint64_t i = key1; i <= key2; i++)
        get(i);
}