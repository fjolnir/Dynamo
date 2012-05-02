#include "GLMath/GLMath.h"
#include "object.h"

#ifndef _SOUND_H_
#define _SOUND_H_

extern Class_t Class_Sound;
// Sound effect (for short sounds that need low latency)
// Encapsulates an OpenAL buffer
extern Class_t Class_SoundEffect;
typedef struct _SoundEffect {
	OBJ_GUTS
	// These properties should only be modified using their setter functions to ensure that
	// the internal state stays up to date with their values.
	vec3_t position;
	bool isLooping;
	float pitch;
	float gain;

	// Platform specific and not guaranteed to exist
	int samples;
	int rate;
	int channels;
	unsigned int buffer;
	unsigned int source;
	unsigned int format;
} SoundEffect_t;

// For longer non latency sensitive sounds, specifically BGM
extern Class_t Class_BackgroundMusic;
typedef struct _BackgroundMusic {
	// Platform specific and not guaranteed to exist
    void *player;
} BackgroundMusic_t;

// Manages an audio device
extern Class_t Class_SoundManager;
typedef struct _SoundManager {
    OBJ_GUTS
	// Platform specific and not guaranteed to exist
	void *device;
	void *context;
} SoundManager_t;

extern SoundEffect_t *sfx_load(const char *aFilename);

extern void sfx_play(SoundEffect_t *aSound);
extern void sfx_stop(SoundEffect_t *aSound);
extern void sfx_toggle(SoundEffect_t *aSound);
extern bool sfx_isPlaying(SoundEffect_t *aSound);

extern void sfx_setPosition(SoundEffect_t *aSound, vec3_t aPos);
extern void sfx_setLooping(SoundEffect_t *aSound, bool aShouldLoop);
extern void sfx_setPitch(SoundEffect_t *aSound, float aPitch);
extern void sfx_setGain(SoundEffect_t *aSound, float aGain);

extern BackgroundMusic_t *bgm_load(const char *aFilename);
extern void bgm_play(BackgroundMusic_t *aBGM);
extern void bgm_stop(BackgroundMusic_t *aBGM);
extern bool bgm_isPlaying(BackgroundMusic_t *aBGM);
extern void bgm_setTime(BackgroundMusic_t *aBGM, float aSeconds);
extern void bgm_setVolume(BackgroundMusic_t *aBGM, float aVolume);

extern SoundManager_t *soundManager_create();
extern bool soundManager_makeCurrent(SoundManager_t *aManager);
#endif
