/*!
	@header Primitive Type Objects
	@abstract
	@discussion Provides Objectified primitive types created in order to be able to memory manage/insert them along with Obj_t's into collections/type check them.

*/
// 
#ifndef _PRIMITIVE_TYPES_
#define _PRIMITIVE_TYPES_

#include "object.h"
#include <string.h>

extern Class_t Class_String;
extern Class_t Class_Number;

/*!
	A string
*/
typedef struct _String {
	OBJ_GUTS
	char *cString;
	long length;
} String_t;
/*!
	Creates a string from the given character array.
*/
String_t *string_create(const char *str, size_t len);

/*!
	A number
*/
typedef struct _Number {
	OBJ_GUTS
	double doubleValue;
} Number_t;

/*!
	Creates a number from the given double.
*/
Number_t *number_create(double num);

#endif
