#include "primitive_types.h"
#include <string.h>
#include <stdlib.h>

// UTF8 Support borrowed from glib
#define UTF8_COMPUTE(Char, Mask, Len)		\
  if (Char < 128)				\
    {						\
      Len = 1;					\
      Mask = 0x7f;				\
    }						\
  else if ((Char & 0xe0) == 0xc0)		\
    {						\
      Len = 2;					\
      Mask = 0x1f;				\
    }						\
  else if ((Char & 0xf0) == 0xe0)		\
    {						\
      Len = 3;					\
      Mask = 0x0f;				\
    }						\
  else if ((Char & 0xf8) == 0xf0)		\
    {						\
      Len = 4;					\
      Mask = 0x07;				\
    }						\
  else if ((Char & 0xfc) == 0xf8)		\
    {						\
      Len = 5;					\
      Mask = 0x03;				\
    }						\
  else if ((Char & 0xfe) == 0xfc)		\
    {						\
      Len = 6;					\
      Mask = 0x01;				\
    }						\
  else						\
    Len = -1;

#define UTF8_LENGTH(Char)			\
  ((Char) < 0x80 ? 1 :				\
   ((Char) < 0x800 ? 2 :			\
    ((Char) < 0x10000 ? 3 :			\
     ((Char) < 0x200000 ? 4 :			\
      ((Char) < 0x4000000 ? 5 : 6)))))


#define UTF8_GET(Result, Chars, Count, Mask, Len)	\
  (Result) = (Chars)[0] & (Mask);			\
  for ((Count) = 1; (Count) < (Len); ++(Count))		\
    {							\
      if (((Chars)[(Count)] & 0xc0) != 0x80)		\
    {						\
      (Result) = -1;				\
      break;					\
    }						\
      (Result) <<= 6;					\
      (Result) |= ((Chars)[(Count)] & 0x3f);		\
    }

#define UNICODE_VALID(Char)			\
  ((Char) < 0x110000 &&				\
   (((Char) & 0xFFFFF800) != 0xD800) &&		\
   ((Char) < 0xFDD0 || (Char) > 0xFDEF) &&	\
   ((Char) & 0xFFFE) != 0xFFFE)


static const char utf8_skip_data[256] = {
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
  2, 2, 2, 2, 2, 2, 2,
  3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5,
  5, 5, 5, 6, 6, 1, 1
};

#define utf8_next_char(p) (char *)((p) + utf8_skip_data[*(unsigned char *)(p)])


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

#pragma mark - String

String_t *string_create(const char *str, size_t len)
{
	dynamo_assert(str != NULL, "Invalid string");
	String_t *self = obj_create_autoreleased(&Class_String);
	self->cString = calloc(1, len+1);
	strncpy((char*)self->cString, str, len);
	self->length = len;

	return self;
}
static void string_destroy(String_t *aString)
{
	free(aString->cString);
}


size_t string_len(String_t *aStr)
{
    long len = 0;
    char *p = (char*)aStr->cString;
    
    if(p == NULL)
        return 0;
    
    while(*p) {
        p = utf8_next_char(p);
        ++len;
    }
    
    return len;
}

unsigned char string_getChar(String_t *aStr, unsigned aIdx)
{
    assert(aIdx < aStr->length);
    
    char *p = (char*)aStr->cString;
    for(int i = 0; i < aStr->length; ++aIdx) {
        p = utf8_next_char(p);
        if(i == aIdx)
            return char_getUnicodeChar(p);
    }
    return -1;
}

int string_findChar(String_t *aStr, unsigned char aChar)
{
    unsigned int len = strlen((char*)aStr->cString);
    
    for(unsigned int i = 0; i < len; ++i)
        if(aStr->cString[i] == aChar) return i;
    
    return -1;
}

bool utfChar_isSpace(unsigned short aChar)
{
    return (aChar >= 0x0009 && aChar <= 0x000D) || aChar == 0x0020 || aChar == 0x0085 || aChar == 0x00A0 || aChar == 0x1680
        || (aChar >= 0x2000 && aChar <= 0x200A) || aChar == 0x2028 || aChar == 0x2029 || aChar == 0x202F
        ||  aChar == 0x205F || aChar == 0x3000;
}

unsigned char char_getUnicodeChar(const char *p)
{
    int i, mask = 0, len;
    unsigned int result;
    unsigned char c = (unsigned char)*p;
    
    UTF8_COMPUTE(c, mask, len);
    if(len == -1)
        return -1;
    UTF8_GET(result, p, i, mask, len);
    
    return result;
}

size_t string_getUnicodeStr(String_t *aStr, unsigned short *aoOut, size_t aMaxLen)
{
    int len = string_len(aStr);
    
    char *buf = aStr->cString;
    aoOut[len] = 0;
    
    for(int i = 0; i < len; ++i)
    {
        aoOut[i] = char_getUnicodeChar(buf);
        buf = utf8_next_char(buf);
    }
    
    return len;
}

#pragma mark - Numbers

Number_t *number_create(double num)
{
	Number_t *self = obj_create_autoreleased(&Class_Number);
	self->doubleValue = num;

	return self;
}

