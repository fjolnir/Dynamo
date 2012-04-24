// Reference counting
#ifndef _OBJECT_H_
#define _OBJECT_H_

// Add OBJ_GUTS at the beginning of a struct type in order to make it a valid, retainable object
typedef void (*Obj_destructor_t)(void *aSelf);
typedef struct {
	long referenceCount;
	Obj_destructor_t destructor;
} _Obj_guts;

typedef void Obj_t;
#define OBJ_GUTS _Obj_guts _guts;

Obj_t *obj_create_autoreleased(int size, Obj_destructor_t aDestructor);
Obj_t *obj_create(int size, Obj_destructor_t aDestructor);
Obj_t *obj_retain(Obj_t *aObj);
void obj_release(Obj_t *aObj);
Obj_t *obj_autorelease(Obj_t *aObj);


#include "linkedlist.h"

typedef LinkedList_t Obj_autoReleasePool_t;

Obj_autoReleasePool_t *autoReleasePool_getGlobal();
Obj_autoReleasePool_t *autoReleasePool_create();
void *autoReleasePool_push(Obj_autoReleasePool_t *aPool, Obj_t *aObj);
void autoReleasePool_drain(Obj_autoReleasePool_t *aPool);

#endif
