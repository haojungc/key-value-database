#include "utils.h"
#include <string.h>
#include <sys/stat.h>

int get_arg_index(int argc, char *argv[], const char *str) {
    for (int i = 1; i < argc; i++)
        if (strcmp(str, argv[i]) == 0)
            return i;
    return -1;
}

void *safe_malloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        fprintf(stderr, "Error: failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }
    return ptr;
}

FILE *safe_fopen(const char *filename, const char *mode) {
    FILE *fp = fopen(filename, mode);
    if (fp == NULL) {
        fprintf(stderr, "Error: failed to open %s\n", filename);
        exit(EXIT_FAILURE);
    }
    return fp;
}

void safe_mkdir(const char *directory, mode_t mode) {
    struct stat st;
    if (stat(directory, &st) == -1) {
        int mkdir_stat = mkdir(directory, mode);
        if (mkdir_stat == -1) {
            fprintf(stderr, "Error: cannot create directory %s", directory);
            exit(EXIT_FAILURE);
        }
    }
}