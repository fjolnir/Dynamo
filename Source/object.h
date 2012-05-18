/*!
	@header Object
	@abstract
	@discussion Provides reference counting and type checking for structs.
*/

#ifndef _OBJECT_H_
#define _OBJECT_H_

#include <stdbool.h>

typedef void (*Obj_destructor_t)(void *aSelf);

/*!
	Provides information about a class of objects
*/
typedef struct {
	char *name;
	long instanceSize;
	Obj_destructor_t destructor;
} Class_t;

/*!
	The core of an object, contains a reference to it's class & it's reference count<br>
	Add OBJ_GUTS at the beginning of a struct type in order to make it a valid, retainable object
*/
typedef struct {
	Class_t *isa;
	long referenceCount;
} _Obj_guts;

/*!
	Represents a reference counted object.
*/
typedef void Obj_t;
#define OBJ_GUTS _Obj_guts _guts;

/*!
	Creates an autoreleased object of the given class
*/
Obj_t *obj_create_autoreleased(Class_t *aClass);
/*!
	Creates an object of the given class
*/
Obj_t *obj_create(Class_t *aClass);
/*!
	Retains an object (increases the reference count by 1)
*/
Obj_t *obj_retain(Obj_t *aObj);
/*!
	Releases an object (decreases the reference count by 1)
*/
void obj_release(Obj_t *aObj);
/*!
	Adds an object to the autorelease pool causing it to be released at the end of the current runloop iteration.
*/
Obj_t *obj_autorelease(Obj_t *aObj);
/*!
	Gets a reference to the class of an object
*/
Class_t *obj_getClass(Obj_t *aObj);
/*!
	Checks if an object is of the given class
*/
bool obj_isClass(Obj_t *aObj, Class_t *aClass);

#include "linkedlist.h"

/*!
	An autorelease pool
*/
typedef LinkedList_t Obj_autoReleasePool_t;

/*!
	Returns the global (default) autorelease pool
*/
Obj_autoReleasePool_t *autoReleasePool_getGlobal();
/*!
	Creates an autorelease pool (And makes it the global pool if one did not exist previously)
*/
Obj_autoReleasePool_t *autoReleasePool_create();
/*!
	Adds an object to the pool
*/
void *autoReleasePool_push(Obj_autoReleasePool_t *aPool, Obj_t *aObj);
/*!
	Drains the pool. (Releases every object in the pool and then empties it)
*/
void autoReleasePool_drain(Obj_autoReleasePool_t *aPool);


// Utility functions to allow obj_retain/release to be used as appliers
void _obj_retain(void *aObj, void *ignored);
void _obj_release(void *aObj, void *ignored);
void _obj_autorelease(void *aObj, void *ignored);

#endif
