#include "utils.h"
#include <stddef.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

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

void *safe_calloc(size_t nmemb, size_t size) {
    void *ptr = calloc(nmemb, size);
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

void safe_fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t read_count = fread(ptr, size, nmemb, stream);
    if (read_count != nmemb) {
        fprintf(stderr, "Error: failed to read from stream\n");
        exit(EXIT_FAILURE);
    }
}

void safe_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t write_count = fwrite(ptr, size, nmemb, stream);
    if (write_count != nmemb) {
        fprintf(stderr, "Error: failed to write to stream\n");
        exit(EXIT_FAILURE);
    }
}

void safe_mkdir(const char *directory, mode_t mode) {
    struct stat st;
    if (stat(directory, &st) == -1) {
        int mkdir_stat = mkdir(directory, mode);
        if (mkdir_stat == -1) {
            fprintf(stderr, "Error: failed to create directory %s", directory);
            exit(EXIT_FAILURE);
        }
    }
}

int file_exists(const char *filename) { return access(filename, F_OK); }