// Basic resizable pointer array

#include "object.h"
#include <stdbool.h>

#ifndef _ARRAY_H_
#define _ARRAY_H_

typedef struct _Array {
	OBJ_GUTS
	int count;
	int capacity;
	void **items;
} Array_t;

extern Array_t *array_create(int aCapacity);
extern void array_push(Array_t *aArray, void *aValue);
extern void array_pop(Array_t *aArray);
extern void array_resize(Array_t *aArray, int aNewCapacity);
extern bool array_containsPtr(Array_t *aArray, void *aPtr);

#endif
