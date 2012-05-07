#include "GLMath/GLMath.h"
#include "object.h"

// Because the different platforms supported are so very different when it comes to audio,
// all audio related types are opaque

#ifndef _SOUND_H_
#define _SOUND_H_

extern Class_t Class_SoundEffect;
// Sound effect (for short sounds that need low latency)
typedef struct _SoundEffect SoundEffect_t;
// For longer non latency sensitive sounds, specifically BGM
extern Class_t Class_BackgroundMusic;
typedef struct _BackgroundMusic BackgroundMusic_t;
// Manages the audio state. Sounds can not be created or played if there is no current sound manager
// Currently you should only create one manager at a time. Creating multiple instances will result in undefined behaviour.
extern Class_t Class_SoundManager;
typedef struct _SoundManager SoundManager_t;

extern SoundEffect_t *sfx_load(const char *aFilename);
extern void sfx_unload(SoundEffect_t *aSound);

extern void sfx_play(SoundEffect_t *aSound);
extern void sfx_stop(SoundEffect_t *aSound);
extern bool sfx_isPlaying(SoundEffect_t *aSound);

extern void sfx_setVolume(SoundEffect_t *aSound, float aVolume);
extern void sfx_setLocation(SoundEffect_t *aSound, vec3_t aPos);
extern void sfx_setLooping(SoundEffect_t *aSound, bool aShouldLoop);
extern void sfx_setPitch(SoundEffect_t *aSound, float aPitch);

extern BackgroundMusic_t *bgm_load(const char *aFilename);
extern void bgm_unload(BackgroundMusic_t *aBGM);

extern void bgm_play(BackgroundMusic_t *aBGM);
extern void bgm_stop(BackgroundMusic_t *aBGM);
extern bool bgm_isPlaying(BackgroundMusic_t *aBGM);
extern void bgm_seek(BackgroundMusic_t *aBGM, float aSeconds);
extern void bgm_setVolume(BackgroundMusic_t *aBGM, float aVolume);

extern SoundManager_t *soundManager_create();
extern bool soundManager_makeCurrent(SoundManager_t *aManager);
#endif
