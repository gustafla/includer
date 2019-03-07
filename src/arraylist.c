#include "arraylist.h"
#include <stdlib.h>
#include <stdio.h>

arraylist_t *arraylist_init(size_t initial_allocated) {
	arraylist_t *list = (arraylist_t*)calloc(1, sizeof(arraylist_t));
	if (!list) return NULL;
	list->list = (void**)calloc(initial_allocated, sizeof(void*));
	if (!list->list) return NULL;
	list->allocated = initial_allocated;
	return list;
}

void arraylist_free(arraylist_t *list) {
	free(list->list);
	free(list);
}

void arraylist_add(arraylist_t *l, void *ptr) {
	if (l->size + 1 > l->allocated) {
		l->allocated = (l->allocated * 3) / 2 + 1;
		l->list = (void**)realloc(l->list, l->allocated * sizeof(void*));
		if (!l->list) {
			fprintf(stderr, "realloc failed in arraylist_add\n");
			exit(EXIT_FAILURE);
		}
	}
	l->list[l->size++] = ptr;
}
