/*!
	@header Primitive Type Objects
	@abstract
	@discussion Provides Objectified primitive types created in order to be able to memory manage/insert them along with Obj_t's into collections/type check them.

*/
// 
#ifndef _PRIMITIVE_TYPES_
#define _PRIMITIVE_TYPES_

#include "object.h"
#include <GLMath/GLMath.h>
#include <string.h>

extern Class_t Class_String;
extern Class_t Class_Number;

/*!
	A string
*/
typedef struct _String {
	OBJ_GUTS
    char *cString;
	long length; // Length of the string data
} String_t;
/*!
	Creates a string from the given character array.
*/
String_t *string_create(const char *str, size_t len);
/*!
 Returns the length of the character string
*/
size_t string_len(String_t *aStr);
/*!
 Returns the index of the first occurrence of aChar in aStr
*/
int string_findChar(String_t *aStr, unsigned char aChar);

/*!
 Determines whether the passed unicode char is a whitespace or not.
*/
bool utfChar_isSpace(unsigned short aChar);
/*!
 Converts a sequence of bytes encoded as UTF-8 to a Unicode character.
*/
unsigned char char_getUnicodeChar(const char *p);

/*!
 Converts a string to Unicode.
*/
size_t string_getUnicodeStr(String_t *aStr, unsigned short *aoOut, size_t aMaxLen);

/*!
	A number
*/
typedef struct _Number {
	OBJ_GUTS
	GLMFloat floatValue;
} Number_t;

/*!
	Creates a number from the given double.
*/
Number_t *number_create(GLMFloat num);

#endif
