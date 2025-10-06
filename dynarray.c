#include "dynarray.h"
#include <stdlib.h>
#include <string.h>

// Create a dynamic array
struct array* arr_create() {
    struct array* arr = malloc(sizeof(struct array));
    arr->cap = 4;
    arr->size = 0;
    arr->items = calloc(arr->cap, sizeof(int));
    return arr;
}

// Insert a number into the array
int arr_insert(struct array* arr, int num) {
    // Resize the array if needed
    if (arr->size == arr->cap) {
        arr->cap *= 2;
        int* temp = calloc(arr->cap, sizeof(int));
        memcpy(temp, arr->items, sizeof(int) * arr->size);
        free(arr->items);
        arr->items = temp;
    }
    arr->items[arr->size] = num;
    arr->size++;
}

// Remove a specified number from the array
int arr_remove(struct array* arr, int num) {
    for (int i = 0; i < arr->size; i++) {
        if (arr->items[i] == num) {
            memmove(&(arr->items[i]), &(arr->items[i+1]), sizeof(int) * (arr->size-i-1));
            break;
        }
    }
    arr->size--;
}

// Free all memory used by the array
void free_array(struct array* arr) {
    free(arr->items);
    free(arr);
}
