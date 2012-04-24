#include "sound.h"
#include "ogg_loader.h"
#include "util.h"

static void sound_destroy(Sound_t *aSound);
static char *_openAlErrorString(int aCode);
static bool _checkForOpenAlError();
static bool _sound_buf_stream(Sound_t *aSound, ALuint aBuffer);
static bool _sound_beginPlayback(Sound_t *aSound);
static bool _sound_update(Sound_t *aSound);

#pragma mark - Sound loading

Sound_t *sound_load(const char *aFilename)
{
#ifndef TARGET_OS_EMBEDDED
	Sound_t *out = obj_create_autoreleased(sizeof(Sound_t), (Obj_destructor_t)&sound_destroy);

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

void sound_destroy(Sound_t *aSound)
{
	if(alIsBuffer(aSound->buffer)) {
		alSourceUnqueueBuffers(aSound->source, 1, &aSound->buffer);		
		alDeleteBuffers(1, &aSound->buffer);
	}
	if(alIsSource(aSound->source))
		alDeleteSources(1, &aSound->source);
}

void sound_setPosition(Sound_t *aSound, vec3_t aPos)
{
	aSound->position = aPos;
	alSource3f(aSound->source, AL_POSITION, aPos.x, aPos.y, aPos.z);
}

void sound_setLooping(Sound_t *aSound, bool aShouldLoop)
{
	aSound->isLooping = aShouldLoop;
	alSourcei(aSound->source, AL_LOOPING, aShouldLoop);
}

void sound_setPitch(Sound_t *aSound, float aPitch)
{
	aSound->pitch = aPitch;
	alSourcef(aSound->source, AL_PITCH, 1.0f);
}

void sound_setGain(Sound_t *aSound, float aGain)
{
	aSound->gain = aGain;
	alSourcef(aSound->source, AL_GAIN, 1.0f);
}
#pragma mark - Sound playing

void sound_play(Sound_t *aSound)
{
	if(sound_isPlaying(aSound))
		return;
	alSourcePlay(aSound->source);
	_checkForOpenAlError();
}
void sound_stop(Sound_t *aSound)
{
	alSourceStop(aSound->source);
	_checkForOpenAlError();
}

bool sound_isPlaying(Sound_t *aSound)
{
	ALenum state;
	alGetSourcei(aSound->source, AL_SOURCE_STATE, &state);
	return (state == AL_PLAYING);
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

