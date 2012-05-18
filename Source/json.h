// Supports string, number, dictionary & array
#ifndef __JSON_H_
#define __JSON_H_

#include "primitive_types.h"
#include "dictionary.h"
#include "array.h"

/*!
	Parses a json string and returns the root object
*/
Obj_t *parseJSON(const char *aJsonStr);

/*!
	Creates a JSON string from an object
*/
bool objToJSON(Obj_t *aObj, char *aoBuf, size_t aBufLen);
#endif
