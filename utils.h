#ifndef UTILS_H
#define UTILS_H
#include <stdio.h>
#include <stdlib.h>

void *safe_malloc(size_t size);
FILE *safe_fopen(const char *filename, const char *mode);

#endif