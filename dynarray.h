#ifndef __DYNARRAY_H
#define __DYNARRAY_H

// Struct to hold a dynamic array of ints (background PIDs)
struct array {
    int size;
    int cap;
    int* items;
};

struct array* arr_create();
int arr_insert(struct array* arr, int num);
int arr_remove(struct array* arr, int num);

void free_array(struct array* arr);

#endif
