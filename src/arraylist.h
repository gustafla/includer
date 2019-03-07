#ifndef ARRAYLIST_H
#define ARRAYLIST_H

#include <stddef.h>

typedef struct {
	void **list;
	size_t size;
	size_t allocated;
} arraylist_t;

arraylist_t *arraylist_init(size_t initial_allocated);
void arraylist_free(arraylist_t *list);
void arraylist_add(arraylist_t *list, void *ptr);

#endif
