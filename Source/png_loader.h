#ifndef _PNGLOADER_H_
#define _PNGLOADER_H_
#include "object.h"

typedef struct {
    OBJ_GUTS
    int height;
    int width;
    bool hasAlpha;
    const unsigned char *data;
} Png_t;

extern Png_t *png_load(const char *aPath);
#endif
