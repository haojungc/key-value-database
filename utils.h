#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>
#include <stdlib.h>

/* Finds str in argv and returns its index. Returns -1 if not found. */
int get_arg_index(int argc, char *argv[], const char *str);
void *safe_malloc(size_t size);
void *safe_calloc(size_t nmemb, size_t size);
FILE *safe_fopen(const char *filename, const char *mode);
void safe_fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
void safe_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
void safe_mkdir(const char *directory, mode_t mode);
int file_exists(const char *filename);

#endif