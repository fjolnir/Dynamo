#include "object.h"
#ifndef _OGGLOADER_H_
#define _OGGLOADER_H_

typedef struct _oggFile {
	OBJ_GUTS
	void *data;
	void *fileHandle;
	int channels;
	int rate;
	int samples;
	int size;
} oggFile_t;

extern oggFile_t *ogg_load(const char *aFilename);

#endif
