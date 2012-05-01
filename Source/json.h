// Supports string, number, dictionary & array
#ifndef __JSON_H_
#define __JSON_H_

#import "primitive_types.h"
#import "dictionary.h"
#import "array.h"

Obj_t *parseJSON(const char *aJsonStr);
bool objToJSON(Obj_t *aObj, char *aoBuf, size_t aBufLen);
#endif
