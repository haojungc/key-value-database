#include "sorting.h"
#include "definition.h"
#include "utils.h"
#include <stdlib.h>

/* Static function prototypes */
static void swap(data_t *d1, data_t *d2);
static void merge(data_t arr[], const int32_t start, const int32_t middle,
                  const int32_t end);

static void swap(data_t *d1, data_t *d2) {
    static data_t tmp;
    tmp = *d1;
    *d1 = *d2;
    *d2 = tmp;
}

static void merge(data_t arr[], const int32_t start, const int32_t middle,
                  const int32_t end) {
    size_t len1 = middle - start + 1;
    size_t len2 = end - middle;
    data_t *a = safe_malloc(len1 * sizeof(data_t));
    data_t *b = safe_malloc(len2 * sizeof(data_t));
    size_t count1 = 0, count2 = 0;

    /* Stores array to be sorted in two buffers */
    for (int i = start; i <= middle; i++)
        a[count1++] = arr[i];
    for (int i = middle + 1; i <= end; i++)
        b[count2++] = arr[i];

    /* Merges the two buffers */
    count1 = count2 = 0;
    for (int i = start; i <= end; i++) {
        if (count1 >= len1) {
            arr[i] = b[count2++];
        } else if (count2 >= len2) {
            arr[i] = a[count1++];
        } else if (a[count1].key < b[count2].key) {
            arr[i] = a[count1++];
        } else {
            arr[i] = b[count2++];
        }
    }
    free(a);
    free(b);
}

void mergesort(data_t arr[], const int32_t start, const int32_t end) {
    if (start >= end - 1) {
        if (start == end - 1)
            if (arr[start].key > arr[end].key)
                swap(&arr[start], &arr[end]);
        return;
    }

    int middle = (start + end) >> 1;
    mergesort(arr, start, middle);
    mergesort(arr, middle + 1, end);
    merge(arr, start, middle, end);
}