#include "ogg_loader.h"
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>
#include <stdlib.h>

static char *_oggErrorString(int aCode);

oggFile_t *ogg_load(char *aFilename)
{
	FILE *handle = fopen(aFilename, "rb");
	if(!handle) return NULL;
	OggVorbis_File *oggStream = malloc(sizeof(OggVorbis_File));
	int result = ov_open(handle, oggStream, NULL, 0);
	if(result > 0) {
		fclose(handle);
		return NULL;
	}
	vorbis_info *vorbisInfo = ov_info(oggStream, -1);
	if(!vorbisInfo)
		fprintf(stderr, "Couldn't get info about vorbis file\n");
	int samples = ov_pcm_total(oggStream, -1);

	int bytes = 2*samples* vorbisInfo->channels;
	void *data = malloc(bytes);

	int bytes_read = 0;
	while(bytes_read < bytes){
		int remain = bytes - bytes_read;
		char *cursor = data + bytes_read;

		long value = ov_read(oggStream, cursor, remain, 0, 2, 1, NULL);
		if(value < 0) {
			fprintf(stderr, "Unable to load sound from %s\n", aFilename);
			free(data);
			return NULL;
		}
		bytes_read += value;
	}

	oggFile_t *out = malloc(sizeof(oggFile_t));
	out->fileHandle = oggStream;
	out->samples = samples;
	out->rate = vorbisInfo->rate;
	out->channels = vorbisInfo->channels;
	out->size = bytes;
	out->data = data;

	return out;
}
extern void ogg_destroy(oggFile_t *aFile)
{
	ov_clear((OggVorbis_File *)aFile->fileHandle);
	free(aFile->data);
	free(aFile);
}
static char *_oggErrorString(int aCode)
{
	switch(aCode) {
		case OV_EREAD:
			return "Read from media.";
		case OV_ENOTVORBIS:
			return "Not Vorbis data.";
		case OV_EVERSION:
			return "Vorbis version mismatch.";
		case OV_EBADHEADER:
			return "Invalid Vorbis header.";
		case OV_EFAULT:
			return "Internal logic fault (bug or heap/stack corruption.";
		default:
			return "Unknown Ogg error.";
	}
}

