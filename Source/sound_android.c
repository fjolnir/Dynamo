// Note: BGM is just a typecast SFX at the moment. I'm not sure if the OpenSL AudioPlayer loads the entire file into memory or not.
// TODO: figure out if it does, and if so, write a streaming implementation of BGM

#ifndef ANDROID
    #error "This is the audio interface for android"
#endif

#include "sound.h"
#include "util.h"
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <dlfcn.h>
#include <string.h>
#include <GLMath/GLMath.h>

#define ANDROID_MAX_SIMUL_SOUNDS 8

struct _SoundEffect {
	OBJ_GUTS
	SoundManager_t *soundManager; // Weak reference
	char *path;
	int channel;
	SLboolean       loaded;
	SLObjectItf     oslPlayerObject;
	SLPlayItf       oslPlayerPlayInterface;
	SLSeekItf       oslPlayerSeekInterface;
	SLVolumeItf     oslPlayerVolumeInterface;
	SLPitchItf      oslPlayerPitchInterface;
	SL3DLocationItf oslPlayer3DLocationInterface;
};

// On android we just use the same mechanism as the sound effect
struct _BackgroundMusic {
	SoundEffect_t *soundEffect;
};

// Manages an audio device
extern Class_t Class_SoundManager;
struct _SoundManager {
    OBJ_GUTS
	bool oslLoadedChannels[ANDROID_MAX_SIMUL_SOUNDS]; // true => Busy, false => Available
	SLObjectItf oslEngineObject;
	SLEngineItf oslEngineInterface;
	SLObjectItf oslOutputMixObject;
};

static SoundManager_t *_CurrentSoundManager = NULL;

static void sfx_destroy(SoundEffect_t *aSound);

static void bgm_destroy(BackgroundMusic_t *aBGM);
static void soundManager_destroy(SoundManager_t *aManager);

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

#pragma mark - Common OpenSL routines

static bool _osl_checkResult(const char *aMsg, SLresult aResult)
{
    if(aResult != SL_RESULT_SUCCESS )
        dynamo_log("%s", aMsg);
    return aResult == SL_RESULT_SUCCESS;
}

static bool _osl_setPlayState(SLPlayItf aPlayInterface, SLuint32 aState)
{
	SLresult result = (*aPlayInterface)->SetPlayState(aPlayInterface, aState);
	return _osl_checkResult("Couldn't set OpenSL play state", result);
}

static SLuint32 _osl_getPlayState(SLPlayItf aPlayInterface)
{
	SLuint32 state;
	SLresult result = (*aPlayInterface)->GetPlayState(aPlayInterface, &state);
	_osl_checkResult("Couldn't get OpenSL play state", result);
	return state;
}


#pragma mark - Sound effects

// Creates an sfx object
SoundEffect_t *sfx_load(const char *aFilename)
{
	dynamo_assert(aFilename != NULL, "Invalid filename");
	dynamo_assert(_CurrentSoundManager != NULL, "Current sound manager not set");
	dynamo_log("loading sound from %s", aFilename);

	SoundEffect_t *out = obj_create_autoreleased(&Class_SoundEffect);
	out->soundManager = _CurrentSoundManager;

	int pathLen = strlen(aFilename);
	out->path = malloc(sizeof(char) * pathLen);
	strncpy(out->path, aFilename, pathLen);
	// Locate a free channel
	int channel = -1;
	for(int i = 0; i < ANDROID_MAX_SIMUL_SOUNDS; ++i) {
		if(_CurrentSoundManager->oslLoadedChannels[i] == false) {
			channel = i;
			break;
		}
	}
	if(channel == -1) {
		dynamo_log("No free channels in current sound manager");
		return NULL;
	}
	out->channel = channel;


	SLDataLocator_URI dataLocator = { SL_DATALOCATOR_URI, (char*)aFilename };
	SLDataFormat_MIME dataFormat = { SL_DATAFORMAT_MIME, NULL, SL_CONTAINERTYPE_UNSPECIFIED };
	SLDataSource dataSource = { &dataLocator, &dataFormat };
	
	SLDataLocator_OutputMix outputMixLocator = { SL_DATALOCATOR_OUTPUTMIX, _CurrentSoundManager->oslOutputMixObject };
	SLDataSink audioSink = { &outputMixLocator, NULL };

	// Create the audio player
	const SLInterfaceID playerIds[3] = { SL_IID_PLAY, SL_IID_VOLUME, SL_IID_SEEK };
	const SLboolean playerReq[3] = { SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE };
	SLresult result = (*_CurrentSoundManager->oslEngineInterface)->CreateAudioPlayer(_CurrentSoundManager->oslEngineInterface, &out->oslPlayerObject,
	                                                                        &dataSource, &audioSink, 3, playerIds, playerReq);
	if(!_osl_checkResult("Couldn't create audio player", result))
		return NULL;
	result = (*out->oslPlayerObject)->Realize(out->oslPlayerObject, SL_BOOLEAN_FALSE);
	if(!_osl_checkResult("Couldn't realize audio player", result))
		return NULL;

	// Get the play interface
	result = (*out->oslPlayerObject)->GetInterface(out->oslPlayerObject, SL_IID_PLAY, &out->oslPlayerPlayInterface);
	if(!_osl_checkResult("Couldn't get audio play interface", result))
		return NULL;
	result = (*out->oslPlayerObject)->GetInterface(out->oslPlayerObject, SL_IID_SEEK, &out->oslPlayerSeekInterface);
	if(!_osl_checkResult("Couldn't get audio seek interface", result))
		return NULL;
	result = (*out->oslPlayerObject)->GetInterface(out->oslPlayerObject, SL_IID_VOLUME, &out->oslPlayerVolumeInterface);
	if(!_osl_checkResult("Couldn't get audio volume interface", result))
		return NULL;
	result = (*out->oslPlayerObject)->GetInterface(out->oslPlayerObject, SL_IID_3DLOCATION, &out->oslPlayer3DLocationInterface);
	if(!_osl_checkResult("3D Audio location unsupported", result))
		out->oslPlayer3DLocationInterface = NULL;
	
	result = (*out->oslPlayerObject)->GetInterface(out->oslPlayerObject, SL_IID_PITCH, &out->oslPlayerPitchInterface);
	if(!_osl_checkResult("Adio pitch not supported", result))
		out->oslPlayerPitchInterface = NULL;


	out->loaded = true;
	_CurrentSoundManager->oslLoadedChannels[channel] = true;
	return out;
}

void sfx_unload(SoundEffect_t *aSound)
{
	if(!aSound->loaded)
		return;
	aSound->loaded = false;
	(*aSound->oslPlayerObject)->Destroy(aSound->oslPlayerObject);
	aSound->soundManager->oslLoadedChannels[aSound->channel] = false;
	aSound->oslPlayerObject = NULL;
	aSound->oslPlayerPlayInterface = NULL;
	aSound->oslPlayerSeekInterface = NULL;
	aSound->oslPlayerVolumeInterface = NULL;
	aSound->soundManager = NULL;
}

void sfx_destroy(SoundEffect_t *aSound)
{
	sfx_unload(aSound);
}

void sfx_setPosition(SoundEffect_t *aSound, vec3_t aPos)
{
	if(!aSound->oslPlayer3DLocationInterface)
		return;
	SLVec3D slVec = { (SLint32)aPos.x, (SLint32)aPos.y, (SLint32)aPos.z };
	SLresult result = (*aSound->oslPlayer3DLocationInterface)->SetLocationCartesian(aSound->oslPlayer3DLocationInterface, &slVec);
	_osl_checkResult("Couldn't set audio position", result);
}

void sfx_setLooping(SoundEffect_t *aSound, bool aShouldLoop)
{
	SLmillisecond duration;
	SLresult result = (*aSound->oslPlayerPlayInterface)->GetDuration(aSound->oslPlayerPlayInterface, &duration);
	if(!_osl_checkResult("Couldn't get duration to loop for", result))
		return;
	result = (*aSound->oslPlayerSeekInterface)->SetLoop(aSound->oslPlayerSeekInterface, aShouldLoop, 0, duration);
	_osl_checkResult("Couldn't set audio position", result);
}

void sfx_setPitch(SoundEffect_t *aSound, float aPitch)
{
	if(!aSound->oslPlayerPitchInterface)
		return;
	SLpermille pitch = aPitch * 1000.0f;
	SLresult result = (*aSound->oslPlayerPitchInterface)->SetPitch(aSound->oslPlayerPitchInterface, pitch);
	_osl_checkResult("Couldn't set audio pitch", result);
}

void _sfx_seek(SoundEffect_t *aSound, float aSeconds)
{
	SLresult result = (*aSound->oslPlayerSeekInterface)->SetPosition(aSound->oslPlayerSeekInterface, (SLmillisecond)(aSeconds*1000), SL_SEEKMODE_FAST);
	_osl_checkResult("Couldn't seek audio", result);
}

float _sfx_getVolume(SoundEffect_t *aSound)
{
	SLmillibel millibels;
	SLresult result = (*aSound->oslPlayerVolumeInterface)->GetVolumeLevel(aSound->oslPlayerVolumeInterface, &millibels);
	_osl_checkResult("Couldn't get audio volume", result);
	return millibels/SL_MILLIBEL_MAX;
}

void sfx_setVolume(SoundEffect_t *aSound, float aVolume)
{
	SLresult result = (*aSound->oslPlayerVolumeInterface)->SetVolumeLevel(aSound->oslPlayerVolumeInterface, (SLmillibel)(aVolume*1000.0));
	_osl_checkResult("Couldn't set audio volume", result);
}

void _sfx_pause(SoundEffect_t *aSound)
{
	_osl_setPlayState(aSound->oslPlayerPlayInterface, SL_PLAYSTATE_PAUSED);
}

void sfx_play(SoundEffect_t *aSound)
{
	dynamo_log("Playing sound %p? %d", aSound, aSound->loaded);
	if(!aSound->loaded) return;
	_osl_setPlayState(aSound->oslPlayerPlayInterface, SL_PLAYSTATE_PLAYING);
}
void sfx_stop(SoundEffect_t *aSound)
{
	if(!aSound->loaded) return;
	_sfx_seek(aSound, 0);
	_osl_setPlayState(aSound->oslPlayerPlayInterface, SL_PLAYSTATE_STOPPED);
}

bool sfx_isPlaying(SoundEffect_t *aSound)
{
	if(!aSound->loaded) return false;
	return _osl_getPlayState(aSound->oslPlayerPlayInterface) == SL_PLAYSTATE_PLAYING;
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

void bgm_setLooping(BackgroundMusic_t *aBGM, bool aLoops)
{
	sfx_setLooping(aBGM->soundEffect, aLoops);
}

#pragma mark - Sound manager (OpenSL)

SoundManager_t *soundManager_create()
{
	void* handle = dlopen("libOpenSLES.so", RTLD_LAZY);
	if (handle == NULL){
		dynamo_log("OpenSLES not available");
		exit(EXIT_FAILURE);
	}
	
	SLresult result;
	SLObjectItf engineObject;

	const SLInterfaceID engineId[] = { SL_IID_ENGINE };
	const SLboolean engineReq[] = { SL_BOOLEAN_TRUE };
	result = slCreateEngine(&engineObject, 0, NULL, 1, engineId, engineReq);
	if(!_osl_checkResult("Couldn't create OpenSL engine", result))
		return NULL;
	result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);
	if(!_osl_checkResult("Couldn't realize OpenSL engine", result))
		return NULL;

	SLEngineItf engineInterface;
	result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineInterface);
	if(!_osl_checkResult("Couldn't get OpenSL interface", result))
		return NULL;

	SLObjectItf outputMixObject;
	const SLInterfaceID mix_ids[1] = {SL_IID_VOLUME};
	const SLboolean mix_req[1] = {SL_BOOLEAN_TRUE};
	result = (*engineInterface)->CreateOutputMix(engineInterface, &outputMixObject, 0, mix_ids, mix_req);
	if(!_osl_checkResult("Couldn't create audio output mix configuration", result))
		return NULL;
	result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
	if(!_osl_checkResult("Couldn't realize audio output mix configuration", result))
		return NULL;

	SoundManager_t *out = obj_create_autoreleased(&Class_SoundManager);
	out->oslEngineObject = engineObject;
	out->oslEngineInterface = engineInterface;
	out->oslOutputMixObject = outputMixObject;

	return out;
}

void soundManager_destroy(SoundManager_t *aManager)
{
	(*aManager->oslEngineObject)->Destroy(aManager->oslEngineObject);
	aManager->oslEngineObject = NULL;
	aManager->oslEngineInterface = NULL;
	aManager->oslOutputMixObject = NULL;
}

bool soundManager_makeCurrent(SoundManager_t *aManager)
{
	if(aManager)             obj_retain(aManager);
	if(_CurrentSoundManager) obj_release(_CurrentSoundManager);
	_CurrentSoundManager = aManager;

	return true;
}
