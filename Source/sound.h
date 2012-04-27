// A simple wrapper for OpenAL
//
#include <OpenAL/AL.h>
#include <OpenAL/alc.h>
#include "GLMath/GLMath.h"
#include "object.h"

#ifndef _SOUND_H_
#define _SOUND_H_

extern Class_t Class_Sound;
// Encapsulates an OpenAL buffer
typedef struct _Sound {
	OBJ_GUTS
	int samples;
	int rate;
	int channels;

	ALuint buffer;
	ALuint source;
	ALenum format;

	// These properties should only be set using their setter functions (Otherwise OpenAL state won't be updated)
	vec3_t position;
	bool isLooping;
	float pitch;
	float gain;
} Sound_t;

// Just manages the OpenAL context&device
typedef struct _SoundManager {
	ALCdevice *device;
	ALCcontext *context;
} SoundManager_t;

extern Sound_t *sound_load(const char *aFilename); // Only supports OGG at the moment

extern void sound_play(Sound_t *aSound);
extern void sound_stop(Sound_t *aSound);
extern void sound_toggle(Sound_t *aSound);
extern bool sound_isPlaying(Sound_t *aSound);

extern void sound_setPosition(Sound_t *aSound, vec3_t aPos);
extern void sound_setLooping(Sound_t *aSound, bool aShouldLoop);
extern void sound_setPitch(Sound_t *aSound, float aPitch);
extern void sound_setGain(Sound_t *aSound, float aGain);

extern SoundManager_t *soundManager_create();
extern void soundManager_destroy(SoundManager_t *aManager);
extern bool soundManager_makeCurrent(SoundManager_t *aManager);
#endif
