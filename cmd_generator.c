#include "definition.h"
#include "utils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[]) {
    if (argc > 9) {
        fprintf(stderr, "Error: too many arguments\n");
        exit(EXIT_FAILURE);
    }

    int64_t total_put, total_get, total_scan;
    char filename[MAX_PATH] = "a.input";

    /* Sets values */
    int put_index = get_arg_index(argc, argv, "-put");
    int get_index = get_arg_index(argc, argv, "-get");
    int scan_index = get_arg_index(argc, argv, "-scan");
    int output_index = get_arg_index(argc, argv, "-output");
    total_put = (put_index == -1) ? 0 : atoll(argv[put_index + 1]);
    total_get = (get_index == -1) ? 0 : atoll(argv[get_index + 1]);
    total_scan = (scan_index == -1) ? 0 : atoll(argv[scan_index + 1]);
    if (output_index != -1)
        strncpy(filename, argv[output_index + 1], MAX_PATH);

    /* Generates cmd file */
    srand(time(NULL));
    FILE *fp = safe_fopen(filename, "w");

    const char table[63] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    int64_t key1, key2;
    char value[VALUE_LENGTH + 1];
    value[VALUE_LENGTH] = '\0';

    /* PUT */
    for (int64_t i = 0; i < total_put; i++) {
        key1 = (((int64_t)rand() << 32) + rand()) & INT64_MAX;
        for (int j = 0; j < VALUE_LENGTH; j++) {
            int index = rand() % 62;
            value[j] = table[index];
        }
        // printf("key = %lu\n"
        //        "value = %s\n",
        //        key1, value);
        fprintf(fp, "PUT %lu %s\n", key1, value);
    }
    /* GET */
    for (int64_t i = 0; i < total_get; i++) {
        key1 = (((int64_t)rand() << 32) + rand()) & INT64_MAX;
        fprintf(fp, "GET %lu\n", key1);
    }
    /* SCAN */
    for (int64_t i = 0; i < total_scan; i++) {
        key1 = (((int64_t)rand() << 32) + rand()) & INT64_MAX;
        key2 = (((int64_t)rand() << 32) + rand()) & INT64_MAX;
        if (key1 > key2) {
            key1 ^= key2;
            key2 ^= key1;
            key1 ^= key2;
        }
        fprintf(fp, "SCAN %lu %lu\n", key1, key2);
    }

    fclose(fp);
    return 0;
}