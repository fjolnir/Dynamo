#ifndef _OGGLOADER_H_
#define _OGGLOADER_H_

typedef struct _oggFile {
	void *data;
	void *fileHandle;
	int channels;
	int rate;
	int samples;
	int size;
} oggFile_t;

extern oggFile_t *ogg_load(const char *aFilename);
extern void ogg_destroy(oggFile_t *aFile);

#endif
