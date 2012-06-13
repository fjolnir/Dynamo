#include "object.h"
#include "util.h"
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
    _Obj_guts *guts = aObj;
	dynamo_log("*** TRIED TO FREE ZOMBIE OBJECT (%p). Break on obj_zombie_error to debug", guts);
	_print_trace();
}

Obj_t *obj_create_autoreleased(Class_t *aClass)
{
	Obj_t *self = obj_create(aClass);
	return obj_autorelease(self);
}

Obj_t *obj_create(Class_t *aClass)
{
	dynamo_assert(aClass != NULL, "Invalid class");

	_Obj_guts *self = calloc(1, aClass->instanceSize);
	self->isa = aClass;
	return obj_retain(self);
}

Obj_t *obj_retain(Obj_t *aObj)
{
	dynamo_assert(aObj != NULL, "Invalid object");
	_Obj_guts *self = aObj;
	__sync_add_and_fetch(&self->referenceCount, 1);
	return aObj;
}

void obj_release(Obj_t *aObj)
{
	dynamo_assert(aObj != NULL, "Invalid object");
	_Obj_guts *self = aObj;
    if(ENABLE_ZOMBIES && self->referenceCount == 0xDEA110CD) {
        obj_zombie_error(self);
        return;
    }
        
	if(__sync_sub_and_fetch(&self->referenceCount, 1) == 0) {
		if(self->isa->destructor)
			self->isa->destructor(aObj);
		if(!ENABLE_ZOMBIES)
			free(self);
		else if(self->referenceCount < 0 || self->referenceCount == 0xDEA110CD)
			obj_zombie_error(self);
        else
            self->referenceCount = 0xDEA110CD;
	}
}

Obj_t *obj_autorelease(Obj_t *aObj)
{
	return autoReleasePool_push(autoReleasePool_getGlobal(), aObj);
}

Class_t *obj_getClass(Obj_t *aObj)
{
	return ((_Obj_guts*)aObj)->isa;
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
	dynamo_assert(aObj != NULL, "Invalid object");
	llist_pushValue(aPool, aObj);
	return aObj;
}
void autoReleasePool_drain(Obj_autoReleasePool_t *aPool)
{
	llist_apply(aPool, &_obj_release, NULL);
	llist_empty(aPool);
}


void _obj_retain(Obj_t *aObj, void *ignored) { obj_retain(aObj); }
void _obj_release(Obj_t *aObj, void *ignored) { obj_release(aObj); }
void _obj_autorelease(Obj_t *aObj, void *ignored) { obj_autorelease(aObj); }

