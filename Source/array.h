// Basic resizable pointer array

#ifndef _ARRAY_H_
#define _ARRAY_H_

#include "object.h"
#include <stdbool.h>

extern Class_t Class_Array;

typedef void (*ArrayInsertionCallback_t)(void *aVal);
typedef void (*ArrayRemovalCallback_t)(void *aVal);

typedef struct _Array {
	OBJ_GUTS
	int count;
	int capacity;
	void **items;
	ArrayInsertionCallback_t insertionCallback;
	ArrayRemovalCallback_t removalCallback;
} Array_t;

extern Array_t *array_create(int aCapacity, ArrayInsertionCallback_t aInsertionCallback, ArrayRemovalCallback_t aRemovalCallback);
extern void array_push(Array_t *aArray, void *aValue);
extern void array_pop(Array_t *aArray);
extern void array_resize(Array_t *aArray, int aNewCapacity);
extern bool array_containsPtr(Array_t *aArray, void *aPtr);
extern void *array_top(Array_t *aArray);

#endif
