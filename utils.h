#ifndef UTILS_H
#define UTILS_H
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

/* Finds str in argv and returns its index. Returns -1 if not found. */
int get_arg_index(int argc, char *argv[], const char *str);

/* malloc with error checking */
void *safe_malloc(size_t size);

/* calloc with error checking */
void *safe_calloc(size_t nmemb, size_t size);

/* fopen with error checking */
FILE *safe_fopen(const char *filename, const char *mode);

/* fread with error checking */
void safe_fread(void *ptr, size_t size, size_t nmemb, FILE *stream);

/* fwrite with error checking */
void safe_fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);

/* mkdir with error checking */
void safe_mkdir(const char *directory, mode_t mode);

/* Checks if a file exists.
 * Returns 0 if the file exists, and -1 otherwise. */
int file_exists(const char *filename);

#endif