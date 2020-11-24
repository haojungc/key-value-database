#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>
#include <stdlib.h>

/* Finds str in argv and returns its index. Returns -1 if not found. */
int get_arg_index(int argc, char *argv[], const char *str);
void *safe_malloc(size_t size);
FILE *safe_fopen(const char *filename, const char *mode);
void safe_mkdir(const char *directory, mode_t mode);

#endif