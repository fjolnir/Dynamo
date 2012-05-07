// Fallback sound API, cross platform but only supports Oggvorbis
#include "sound.h"
#include "ogg_loader.h"
#include "util.h"

struct _SoundEffect {
	OBJ_GUTS
	// These properties should only be modified using their setter functions to ensure that
	// the internal state stays up to date with their values.
	vec3_t position;
	bool isLooping;
	float pitch;

	// Platform specific and not guaranteed to exist
	void *handle;
	int samples;
	int rate;
	int channels;
	unsigned int buffer;
	unsigned int source;
	unsigned int format;
};
struct _BackgroundMusic {
    void *player;
};
struct _SoundManager {
    OBJ_GUTS
    ALCcontext *context;
	ALCdevice *device;
};


static void sound_destroy(SoundEffect_t *aSound);
static char *_openAlErrorString(int aCode);
static bool _checkForOpenAlError();
static bool _sound_buf_stream(SoundEffect_t *aSound, ALuint aBuffer);
static bool _sound_beginPlayback(SoundEffect_t *aSound);
static bool _sound_update(SoundEffect_t *aSound);

Class_t Class_SoundEffect = {
	"SoundEffect",
	sizeof(SoundEffect_t),
	(Obj_destructor_t)&sfx_destroy
};

Class_t Class_BackgroundMusic = {
	"BackgroundMusic",
	sizeof(BackgroundMusic_t),
	(Obj_destructor_t)&bgm_destroy
};

Class_t Class_SoundManager = {
	"SoundManager",
	sizeof(SoundManager_t),
	(Obj_destructor_t)&soundManager_destroy
};

#pragma mark - Sound loading

SoundEffect_t *sound_load(const char *aFilename)
{
#ifndef TARGET_OS_EMBEDDED
	SoundEffect_t *out = obj_create_autoreleased(&Class_Sound);

	oggFile_t *oggFile = ogg_load(aFilename);
	if(!oggFile) {
		debug_log("Failed to load audio file");
		return NULL;
	}
	out->format = out->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
	alGenBuffers(1, &out->buffer);
	alGenSources(1, &out->source);
	_checkForOpenAlError();
	alBufferData(out->buffer, out->format, oggFile->data, oggFile->size, oggFile->rate);
	_checkForOpenAlError();

	alSourcei(out->source, AL_BUFFER,  out->buffer);
	sound_setPosition(out, GLMVec3_zero);
	sound_setLooping(out, false);
	sound_setPitch(out, 1.0f);
	sound_setGain(out, 1.0f);
	alSource3f(out->source, AL_VELOCITY,        0.0, 0.0, 0.0);
	alSource3f(out->source, AL_DIRECTION,       0.0, 0.0, 0.0);
	alSourcef (out->source, AL_ROLLOFF_FACTOR,  0.0          );
	alSourcei (out->source, AL_SOURCE_RELATIVE, AL_TRUE      );

	out->channels = oggFile->channels;
	out->rate = oggFile->rate;
	out->samples = oggFile->samples;

	return out;
#endif
    return NULL;
}

void sfx_unload(SoundEffect_t *aSound)
{
	if(alIsBuffer(aSound->buffer)) {
		alSourceUnqueueBuffers(aSound->source, 1, &aSound->buffer);
		alDeleteBuffers(1, &aSound->buffer);
	}
	if(alIsSource(aSound->source))
		alDeleteSources(1, &aSound->source);
}

void sfx_destroy(SoundEffect_t *aSound)
{
	sfx_unload(aSound);
}

void sfx_setVolume(SoundEffect_t *aSound, float aVolume)
{
    alSourcef(aSound->source, AL_GAIN, aVolume);
}

void sfx_setLocation(SoundEffect_t *aSound, vec3_t aPos)
{
	aSound->position = aPos;
	alSource3f(aSound->source, AL_POSITION, aPos.x, aPos.y, aPos.z);
}

void sfx_setLooping(SoundEffect_t *aSound, bool aShouldLoop)
{
	aSound->isLooping = aShouldLoop;
	alSourcei(aSound->source, AL_LOOPING, aShouldLoop);
}

void sfx_setPitch(SoundEffect_t *aSound, float aPitch)
{
	aSound->pitch = aPitch;
	alSourcef(aSound->source, AL_PITCH, 1.0f);
}

void sfx_play(SoundEffect_t *aSound)
{
	if(sfx_isPlaying(aSound))
		sfx_stop(aSound);
	alSourcePlay(aSound->source);
	_checkForOpenAlError();
}
void sfx_stop(SoundEffect_t *aSound)
{
	alSourceStop(aSound->source);
	_checkForOpenAlError();
}

bool sfx_isPlaying(SoundEffect_t *aSound)
{
	ALenum state;
	alGetSourcei(aSound->source, AL_SOURCE_STATE, &state);
	return (state == AL_PLAYING);
}

#pragma mark - BGM

BackgroundMusic_t *bgm_load(const char *aFilename)
{
	BackgroundMusic_t *out = obj_create_autoreleased(&Class_BackgroundMusic);
	out->soundEffect = sfx_load(aFilename);
	if(!out->soundEffect)
		return NULL;
	obj_retain(out->soundEffect);
	return out;
}

void bgm_unload(BackgroundMusic_t *aBGM)
{
	sfx_unload(aBGM->soundEffect);
}

static void bgm_destroy(BackgroundMusic_t *aBGM)
{
	bgm_unload(aBGM);
	obj_release(aBGM->soundEffect);
}

void bgm_play(BackgroundMusic_t *aBGM)
{
	sfx_play(aBGM->soundEffect);
}
void bgm_stop(BackgroundMusic_t *aBGM)
{
	sfx_stop(aBGM->soundEffect);
}
bool bgm_isPlaying(BackgroundMusic_t *aBGM)
{
	return sfx_isPlaying(aBGM->soundEffect);
}
void bgm_seek(BackgroundMusic_t *aBGM, float aSeconds)
{
	_sfx_seek(aBGM->soundEffect, aSeconds);
}
void bgm_setVolume(BackgroundMusic_t *aBGM, float aVolume)
{
	sfx_setVolume(aBGM->soundEffect, aVolume);
}


#pragma mark - Sound manager

SoundManager_t *soundManager_create()
{
	SoundManager_t *out = malloc(sizeof(SoundManager_t));
	out->device = alcOpenDevice(NULL);
	if(!out->device) return NULL;
	out->context = alcCreateContext(out->device, NULL);
	if(!out->context){
		alcCloseDevice(out->device);
		free(out);
		return NULL;
	}
	if(!soundManager_makeCurrent(out)) {
		alcDestroyContext(out->context);
		alcCloseDevice(out->device);
		free(out);
		return NULL;
	}
	return out;
}

void soundManager_destroy(SoundManager_t *aManager)
{
	alcDestroyContext(aManager->context);
	alcCloseDevice(aManager->device);
	free(aManager);
}

bool soundManager_makeCurrent(SoundManager_t *aManager)
{
	return alcMakeContextCurrent(aManager->context);
}

#pragma mark - Utilities

static char *_openAlErrorString(int aCode)
{
		switch(aCode) {
		case AL_INVALID_NAME:
			return "Read from media.";
		case AL_INVALID_ENUM:
			return "Invalid parameter passed to AL call.";
		case AL_INVALID_VALUE:
			return "Invalid enum parameter value.";
		case AL_INVALID_OPERATION:
			return "Illegal OpenAL call.";
		default:
			return "Unknown OpenAL error.";
	}
}

static bool _checkForOpenAlError()
{
	int error = alGetError();
	if(error != AL_NO_ERROR) {
		debug_log("OpenAL error occurred(%d): %s", error, _openAlErrorString(error));
		return true;
	}
	return false;
}

