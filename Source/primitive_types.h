// Objectified primitive types created in order to be able to memory manage/insert them along with Obj_t's into collections

#ifndef _PRIMITIVE_TYPES_
#define _PRIMITIVE_TYPES_

#include "object.h"

extern Class_t Class_String;
extern Class_t Class_Number;

typedef struct _String {
	OBJ_GUTS
	char *cString;
	long length;
} String_t;

String_t *string_create(const char *str);

typedef struct _Number {
	OBJ_GUTS
	double doubleValue;
} Number_t;

Number_t *number_create(double num);

#endif
