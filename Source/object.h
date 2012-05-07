// Reference counting
#ifndef _OBJECT_H_
#define _OBJECT_H_

#include <stdbool.h>

typedef void (*Obj_destructor_t)(void *aSelf);

typedef struct {
	char *name;
	long instanceSize;
	Obj_destructor_t destructor;
} Class_t;

// Add OBJ_GUTS at the beginning of a struct type in order to make it a valid, retainable object
typedef struct {
	Class_t *isa;
	long referenceCount;
} _Obj_guts;

typedef void Obj_t;
#define OBJ_GUTS _Obj_guts _guts;

Obj_t *obj_create_autoreleased(Class_t *aClass);
Obj_t *obj_create(Class_t *aClass);
Obj_t *obj_retain(Obj_t *aObj);
void obj_release(Obj_t *aObj);
Obj_t *obj_autorelease(Obj_t *aObj);
Class_t *obj_getClass(Obj_t *aObj);
bool obj_isClass(Obj_t *aObj, Class_t *aClass);

#include "linkedlist.h"

typedef LinkedList_t Obj_autoReleasePool_t;

Obj_autoReleasePool_t *autoReleasePool_getGlobal();
Obj_autoReleasePool_t *autoReleasePool_create();
void *autoReleasePool_push(Obj_autoReleasePool_t *aPool, Obj_t *aObj);
void autoReleasePool_drain(Obj_autoReleasePool_t *aPool);

#endif
