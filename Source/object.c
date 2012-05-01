#include "object.h"
#include "util.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define ENABLE_ZOMBIES (0)

#ifdef __APPLE__
#include <execinfo.h>
static void _print_trace(void)
{
	void *array[8];
	size_t size;
	char **strings;
	size_t i;

	size = backtrace(array, 8);
	strings = backtrace_symbols(array, size);

	for(i = 0; i < size; i++)
		printf("%s\n", strings[i]);
	free (strings);
}
#else
static void _print_trace(void)
{
	// Backtraces only supported on apple devices for now.
}
#endif

void obj_zombie_error(Obj_t *aObj)
{
	debug_log("*** TRIED TO FREE ZOMBIE OBJECT (%p). Break on obj_zombie_error to debug", aObj);
	_print_trace();
}

Obj_t *obj_create_autoreleased(Class_t *aClass)
{
	Obj_t *self = obj_create(aClass);
	return obj_autorelease(self);
}

Obj_t *obj_create(Class_t *aClass)
{
	assert(aClass != NULL);

	_Obj_guts *self = calloc(1, aClass->instanceSize);
	self->class = aClass;
	return obj_retain(self);
}

Obj_t *obj_retain(Obj_t *aObj)
{
	assert(aObj != NULL);
	_Obj_guts *self = aObj;
	__sync_add_and_fetch(&self->referenceCount, 1);
	return aObj;
}
void obj_release(Obj_t *aObj)
{
	assert(aObj != NULL);
	_Obj_guts *self = aObj;
	if(__sync_sub_and_fetch(&self->referenceCount, 1) == 0 && !ENABLE_ZOMBIES) {
		if(self->class->destructor)
			self->class->destructor(aObj);
		if(!ENABLE_ZOMBIES)
			free(self);
		else if (self->referenceCount < 0)
			obj_zombie_error(self);
	}
}

Obj_t *obj_autorelease(Obj_t *aObj)
{
	return autoReleasePool_push(autoReleasePool_getGlobal(), aObj);
}

Class_t *obj_getClass(Obj_t *aObj)
{
	return ((_Obj_guts*)aObj)->class;
}

bool obj_isClass(Obj_t *aObj, Class_t *aClass)
{
	return obj_getClass(aObj) == aClass;
}

static void autoReleasePool_destroy(void *aPool)
{
	autoReleasePool_drain(aPool);
}

static Class_t Class_autoReleasePool = {
	"AutoreleasePool",
	sizeof(Obj_autoReleasePool_t),
	(Obj_destructor_t)&autoReleasePool_destroy
};

Obj_autoReleasePool_t *autoReleasePool_create()
{
	return obj_create(&Class_autoReleasePool);
}
static Obj_autoReleasePool_t *globalPool;
Obj_autoReleasePool_t *autoReleasePool_getGlobal()
{
	if(!globalPool)
		globalPool = autoReleasePool_create();
	return globalPool;
}
void *autoReleasePool_push(Obj_autoReleasePool_t *aPool, void *aObj)
{
	assert(aObj != NULL);
	llist_pushValue(aPool, aObj);
	return aObj;
}
void autoReleasePool_drain(Obj_autoReleasePool_t *aPool)
{
	llist_apply(aPool, &obj_release);
	llist_empty(aPool);
}

