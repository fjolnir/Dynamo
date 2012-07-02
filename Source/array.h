/*!
    @header Array
    @abstract
    @discussion Implementation of a resizable pointer array.
*/
#ifndef _ARRAY_H_
#define _ARRAY_H_

#include "object.h"
#include <stdbool.h>

extern Class_t Class_Array;

typedef struct _Array {
    OBJ_GUTS
    int count;
    int capacity;
    void **items;
    InsertionCallback_t insertionCallback;
    RemovalCallback_t removalCallback;
} Array_t;


/*!
    Creates an array
    @param aCapacity The initial capacity of the array (The array is automatically resized if you exceed it)
    @param aInsertionCallback A callback function that is called when an item is inserted into the array
    @param aRemovalCallback A callback function that is called when an item is inserted into the array
        Array_t *anArray = array_create(16, &obj_retain, &obj_release);
*/
extern Array_t *array_create(int aCapacity, InsertionCallback_t aInsertionCallback, RemovalCallback_t aRemovalCallback);

/*!
Pushes an item onto the end of an array as if it were a stack
*/
extern void array_push(Array_t *aArray, void *aValue);
/*!
    Pops the last item off an array as if it were a stack
*/
extern void array_pop(Array_t *aArray);
/*!
    Adjusts the capacity of an array
*/
extern void array_resize(Array_t *aArray, int aNewCapacity);
/*!
    Checks if the array contains a given item
*/
extern bool array_containsPtr(Array_t *aArray, void *aPtr);
/*!
    Returns the last item in the array
*/
extern void *array_top(Array_t *aArray);
/*!
    Returns the item at a given index
*/
extern void *array_get(Array_t *aArray, unsigned int aIdx);

#endif
