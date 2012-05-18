/*!
	@header PNG Loader
	@abstract
	@discussion Provides loading of PNGs.
*/

#ifndef _PNGLOADER_H_
#define _PNGLOADER_H_
#include "object.h"

/*!
	Representation of a png image.
*/
typedef struct {
    OBJ_GUTS
    int height;
    int width;
    bool hasAlpha;
    const unsigned char *data;
} Png_t;

/*!
	Loads a png from the given path
*/
extern Png_t *png_load(const char *aPath);
#endif
