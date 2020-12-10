#include "database.h"
#include "bloomfilter.h"
#include "bptree.h"
#include "definition.h"
#include "utils.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

/* macros */
#define MAX_FILENAME_LENGTH 50
#define MAX_METADATA 100
#define MAX_KEY 2000000

/* static variables */
static FILE *fp = NULL;
static const char *dir_path = "storage";
static const char *meta_file_path = "storage/meta";
static bloomfilter_t bf;
static char bf_file_path[MAX_FILENAME_LENGTH];
static bptree_t bptree;
static metadata_t metatable[MAX_METADATA];
static size_t meta_count = 0;
static int32_t loaded_file = -1;
static size_t key_count = 0;

/* static function prototypes */
static void close();
static void set_output_filename(const char *filename);
static void put(const uint64_t key, char *value);
static void get(const uint64_t key);
static void scan(const uint64_t key1, const uint64_t key2);
static void load_metatable();
static void save_metatable();
/* Saves the first half of the buffer to the file and loads the file found from
 * the metatable. */
static void swap_files(metadata_t *metadata);

/* static functions */
static void close() {
    puts("closing database ...");

    bf.save(bf_file_path);
    bf.free();

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

    if (fp != NULL)
        fclose(fp);
}

static void set_output_filename(const char *filename) {
    fp = safe_fopen(filename, "w");
}

static void put(const uint64_t key, char *value) {
    bf.add(key);

    /* Looks up the metatable */
    /* TODO: inefficient if the input keys are distributed randomly, sort the
     * keys before inserting into B+ tree */
    for (int i = 0; i < meta_count; i++) {
        bool need_to_swap = key >= metatable[i].start_key &&
                            key <= metatable[i].end_key &&
                            metatable[i].file_number != loaded_file;
        if (need_to_swap) {
            swap_files(&metatable[i]);
            break;
        }
    }

    int_fast8_t key_increment = bptree.insert(key, value);
    key_count += key_increment;
    if (key_count >= MAX_KEY) {
        /* Splits B+ tree into two parts and saves them separately */
        static char file1[MAX_FILENAME_LENGTH + 1];
        static char file2[MAX_FILENAME_LENGTH + 1];
        size_t file_number1 = (loaded_file == -1) ? meta_count++ : loaded_file;
        size_t file_number2 = meta_count++;
        metatable[file_number1].file_number = file_number1;
        metatable[file_number2].file_number = file_number2;
        snprintf(file1, MAX_FILENAME_LENGTH, "%s/%lu", dir_path, file_number1);
        snprintf(file2, MAX_FILENAME_LENGTH, "%s/%lu", dir_path, file_number2);
        bptree.split_and_save(&metatable[file_number1],
                              &metatable[file_number2], file1, file2);
        loaded_file = -1;
        key_count = 0;
    }
}

static void get(const uint64_t key) {
    int_fast8_t result = bf.lookup(key);
    /* Not found */
    if (result == -1) {
        /* Write to output file */
        return;
    }
    /* Search key in database */
}

static void scan(const uint64_t key1, const uint64_t key2) {
    /* parallelizable? */
    for (uint64_t i = key1; i <= key2; i++)
        get(i);
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

    /* Updates information */
    loaded_file = metadata->file_number;
    key_count = metadata->total_keys;
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
}