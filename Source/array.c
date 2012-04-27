#include "array.h"
#include <stdlib.h>

static void array_destroy(Array_t *aArray);
static Class_t Class_Array = {
	"Array",
	sizeof(Array_t),
	(Obj_destructor_t)&array_destroy
};

Array_t *array_create(int aCapacity)
{
	Array_t *out = obj_create_autoreleased(&Class_Array);
	out->capacity = aCapacity ? aCapacity : 4;
	out->items = calloc(aCapacity, sizeof(void*));
	out->count = 0;

	return out;
}

void array_destroy(Array_t *aArray)
{
	free(aArray->items);
}

void array_push(Array_t *aArray, void *aValue)
{
	if(aArray->count+1 >= aArray->capacity)
		array_resize(aArray, aArray->capacity*2);
	aArray->items[aArray->count++] = aValue;
}

void array_pop(Array_t *aArray)
{
	if(--aArray->count <= aArray->capacity/2)
		array_resize(aArray, aArray->capacity/2);
}

void array_resize(Array_t *aArray, int aNewCapacity)
{
	aArray->capacity = aNewCapacity;
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
