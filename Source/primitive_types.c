#include "primitive_types.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>

static void string_destroy(String_t *aString);

Class_t Class_String = {
	"String",
	sizeof(String_t),
	(Obj_destructor_t)&string_destroy
};
Class_t Class_Number = {
	"Number",
	sizeof(Number_t),
	NULL
};

String_t *string_create(const char *str, size_t len)
{
	assert(str != NULL);
	String_t *self = obj_create_autoreleased(&Class_String);
	self->cString = calloc(1, len+1);
	strncpy(self->cString, str, len);
	self->length = len;

	return self;
}
static void string_destroy(String_t *aString)
{
	free(aString->cString);
}

Number_t *number_create(double num)
{
	Number_t *self = obj_create_autoreleased(&Class_Number);
	self->doubleValue = num;

	return self;
}

