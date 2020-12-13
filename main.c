#include "database.h"
#include "definition.h"
#include "utils.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CMD_LENGTH 200

static void manage_database(const char *f_in);

int main(int argc, char *argv[]) {
    /* Checks the number of command-line arguments */
    if (argc != 2) {
        fprintf(stderr,
                "Error: %s\n"
                "Format: %s <filename>\n",
                (argc > 2) ? "too many arguments" : "too few arguments",
                argv[0]);
        exit(EXIT_FAILURE);
    }

    char f_in[MAX_PATH + 1];
    strncpy(f_in, argv[1], MAX_PATH);

    manage_database(f_in);

    return 0;
}

static void manage_database(const char *f_in) {
    /* Checks file extension */
    char *dot = strrchr(f_in, '.');
    if (dot == NULL) {
        fprintf(stderr,
                "Error: invalid input file name\n"
                "file extension of the input file should be \".input\"\n");
        exit(EXIT_FAILURE);
    }
    char *extension = strstr(dot, ".input");
    if (extension == NULL) {
        fprintf(stderr,
                "Error: invalid input file name\n"
                "file extension of the input file should be \".input\"\n");
        exit(EXIT_FAILURE);
    }

    /* Sets the output filename */
    char f_out[MAX_PATH + 1];
    char *slash = strrchr(f_in, '/');
    strncpy(f_out, (slash == NULL) ? f_in : slash + 1, MAX_PATH);
    dot = strrchr(f_out, '.');
    strncpy(dot, ".output", 8);

    database_t db;
    init_database(&db);

    FILE *fp_in = safe_fopen(f_in, "r");

    char cmd[MAX_CMD_LENGTH];
    bool output_is_set = false;
    while (fgets(cmd, MAX_CMD_LENGTH, fp_in) != NULL) {
        // printf("%s", cmd);
        uint64_t key1, key2;
        static char value[VALUE_LENGTH + 1];
        if (sscanf(cmd, "PUT %lu %s", &key1, value) == 2) {
            // printf("PUT %lu %s\n", key1, value);
            db.put(key1, value);
        } else if (sscanf(cmd, "GET %lu", &key1) == 1) {
            if (output_is_set == false) {
                db.set_output_filename(f_out);
                output_is_set = true;
            }
            // printf("GET %lu\n", key1);
            db.get(key1);
        } else if (sscanf(cmd, "SCAN %lu %lu", &key1, &key2) == 2) {
            if (output_is_set == false) {
                db.set_output_filename(f_out);
                output_is_set = true;
            }
            // printf("SCAN %lu %lu\n", key1, key2);
            db.scan(key1, key2);
        } else {
            fprintf(stderr, "Error: \"%s\" is an invalid command\n", cmd);
        }
    }
    fclose(fp_in);
    db.close();
}
