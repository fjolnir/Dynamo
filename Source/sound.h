#include "GLMath/GLMath.h"
#include "object.h"

// Because the different platforms supported are so very different when it comes to audio,
// all audio related types are opaque

#ifndef _SOUND_H_
#define _SOUND_H_

extern Class_t Class_SoundEffect;
/*!
	Sound effect (for short sounds that need low latency)
*/
typedef struct _SoundEffect SoundEffect_t;
/*!
	Background music (For longer non latency sensitive sounds, specifically BGM)
*/
typedef struct _BackgroundMusic BackgroundMusic_t;
extern Class_t Class_BackgroundMusic;
/*!
	Manages the audio state. Sounds can not be created or played if there is no current sound manager.<br>
	Currently you should only create one manager at a time. Creating multiple instances will result in undefined behaviour.
*/
typedef struct _SoundManager SoundManager_t;
extern Class_t Class_SoundManager;

/*!
	Loads a sound effect
*/
extern SoundEffect_t *sfx_load(const char *aFilename);
/*!
	Unloads a sound effect
*/
extern void sfx_unload(SoundEffect_t *aSound);

/*!
	Plays a sound effect.
*/
extern void sfx_play(SoundEffect_t *aSound);
/*!
	Stops playback of a sound effect.
*/
extern void sfx_stop(SoundEffect_t *aSound);
/*!
	Checks whether a sound effect is playing or not.
*/
extern bool sfx_isPlaying(SoundEffect_t *aSound);

/*!
	Sets the volume of a sound effect.
*/
extern void sfx_setVolume(SoundEffect_t *aSound, float aVolume);
/*!
	Sets the 3D location of a sound effect.
*/
extern void sfx_setLocation(SoundEffect_t *aSound, vec3_t aPos);
/*!
	Sets whether or not a sound effect should loop or not.
*/
extern void sfx_setLooping(SoundEffect_t *aSound, bool aShouldLoop);
/*!
	Sets the pitch of a sound effect.
*/
extern void sfx_setPitch(SoundEffect_t *aSound, float aPitch);

/*!
	Loads BGM.
*/
extern BackgroundMusic_t *bgm_load(const char *aFilename);
/*!
	Unloads BGM.
*/
extern void bgm_unload(BackgroundMusic_t *aBGM);

/*!
	Starts playback of a BGM object.
*/
extern void bgm_play(BackgroundMusic_t *aBGM);
/*!
	Stops playback of a BGM object.
*/
extern void bgm_stop(BackgroundMusic_t *aBGM);
/*!
	Checks whether or not a BGM object is playing or not
*/
extern bool bgm_isPlaying(BackgroundMusic_t *aBGM);
/*!
	Sets the current playback position of a BGM object
*/
extern void bgm_seek(BackgroundMusic_t *aBGM, float aSeconds);
/*!
	Sets the volume of a BGM object.
*/
extern void bgm_setVolume(BackgroundMusic_t *aBGM, float aVolume);

/*!
	Creates a sound manager.
*/
extern SoundManager_t *soundManager_create();
/*!
	Sets a sound manager as the current manager.
*/
extern bool soundManager_makeCurrent(SoundManager_t *aManager);
#endif
