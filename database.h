#ifndef DATABASE_H
#define DATABASE_H
#include <stdint.h>

typedef struct database {
    /* Closes the database */
    void (*close)();
    void (*set_output_filename)(const char *filename);
    void (*put)(const uint64_t key, char *value);
    void (*get)(const uint64_t key);
    void (*scan)(const uint64_t start_key, const uint64_t end_key);
} database_t;

void init_database(database_t *db);

#endif