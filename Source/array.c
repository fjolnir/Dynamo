#include "array.h"
#include <stdlib.h>
#include <util.h>

void array_destroy(Array_t *aArray);
Class_t Class_Array = {
    "Array",
    sizeof(Array_t),
    (Obj_destructor_t)&array_destroy
};

Array_t *array_create(int aCapacity, InsertionCallback_t aInsertionCallback, RemovalCallback_t aRemovalCallback)
{
    Array_t *out = obj_create_autoreleased(&Class_Array);
    out->capacity = aCapacity ? aCapacity : 4;
    out->items = malloc(aCapacity*sizeof(void*));
    out->count = 0;
    out->insertionCallback = aInsertionCallback;
    out->removalCallback = aRemovalCallback;

    return out;
}

void array_destroy(Array_t *aArray)
{
    if(aArray->removalCallback) {
        for(int i = 0; i < aArray->count; ++i)
            aArray->removalCallback(aArray->items[i]);
    }
    free(aArray->items);
}

void array_push(Array_t *aArray, void *aValue)
{
    if(aArray->insertionCallback)
        aArray->insertionCallback(aValue);

    if(aArray->count+1 >= aArray->capacity)
        array_resize(aArray, aArray->capacity*2);
    aArray->items[aArray->count++] = aValue;
}

void array_pop(Array_t *aArray)
{
    dynamo_assert(aArray->count > 0, "Invalid array");
    if(aArray->removalCallback)
        aArray->removalCallback(aArray->items[aArray->count-1]);

    if(--aArray->count <= aArray->capacity/2)
        array_resize(aArray, aArray->capacity/2);
}

void array_resize(Array_t *aArray, int aNewCapacity)
{
  int oldCapacity = aArray->capacity;
    aArray->capacity = aNewCapacity;
  // We don't need to bother shrinking the array if the capacity is insignificantly small
  if(aArray->capacity > oldCapacity || oldCapacity > 32)
    aArray->items = realloc(aArray->items, sizeof(void*)*aNewCapacity);
}

bool array_containsPtr(Array_t *aArray, void *aPtr)
{
    for(int i = 0; i < aArray->count; ++i) {
        if(aArray->items[i] == aPtr)
            return true;
    }
    return false;
}

extern void *array_top(Array_t *aArray)
{
    dynamo_assert(aArray->count > 0, "Invalid array");
    return aArray->items[aArray->count - 1];
}

extern void *array_get(Array_t *aArray, unsigned int aIdx)
{
    dynamo_assert(aIdx < aArray->count, "Invalid array");
    return aArray->items[aIdx];
}
