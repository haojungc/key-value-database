#include "database.h"
#include "utils.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FILENAME_LENGTH 50
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

    char f_in[MAX_FILENAME_LENGTH];
    strncpy(f_in, argv[1], MAX_FILENAME_LENGTH);

    manage_database(f_in);

    return 0;
}

static void manage_database(const char *f_in) {
    /* Sets the output file name */
    char f_out[MAX_FILENAME_LENGTH];
    strncpy(f_out, f_in, MAX_FILENAME_LENGTH);
    /* File extension check */
    char *dot = strrchr(f_out, '.');
    char *extension = strstr(dot, ".input");
    if (dot == NULL || extension == NULL) {
        fprintf(stderr,
                "Error: invalid input file name\n"
                "file extension of the input file should be \".input\"\n");
        exit(EXIT_FAILURE);
    }
    strncpy(dot, ".output", 8);
    init_database();

    FILE *fp_in = safe_fopen(f_in, "r");

    char cmd[MAX_CMD_LENGTH];
    bool output_is_set = false;
    while (fgets(cmd, MAX_CMD_LENGTH, fp_in) != NULL) {
        // printf("%s", cmd);
        uint64_t key1, key2;
#define VALUE_LENGTH 128
        char value[VALUE_LENGTH + 1];
#undef VALUE_LENGTH
        if (sscanf(cmd, "PUT %lu %s", &key1, value) == 2) {
            // printf("PUT %lu %s\n", key1, value);
            put(key1, value);
        } else if (sscanf(cmd, "GET %lu", &key1) == 1) {
            if (output_is_set == false) {
                set_output_filename(f_out);
                output_is_set = true;
            }
            // printf("GET %lu\n", key1);
            get(key1);
        } else if (sscanf(cmd, "SCAN %lu %lu", &key1, &key2) == 2) {
            if (output_is_set == false) {
                set_output_filename(f_out);
                output_is_set = true;
            }
            // printf("SCAN %lu %lu\n", key1, key2);
            // scan(key1, key2);
        } else {
            fprintf(stderr, "Error: \"%s\" is an invalid command\n", cmd);
        }
    }
    fclose(fp_in);
    close_database();
}
