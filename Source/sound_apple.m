#include "sound.h"
#include "util.h"

#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CFURL.h>
#include <OpenAL/AL.h>
#include <OpenAL/alc.h>

// TODO: Soften error handling

#ifndef __APPLE__
    #error "This is the audio interface for apple platforms"
#endif

#ifdef __APPLE__
	#import <AVFoundation/AVFoundation.h>
#endif

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

static void sfx_destroy(SoundEffect_t *aSound);
static char *_openAlErrorString(int aCode);
static bool _checkForOpenAlError();
static bool _sound_buf_stream(SoundEffect_t *aSound, ALuint aBuffer);
static bool _sound_beginPlayback(SoundEffect_t *aSound);
static bool _sound_update(SoundEffect_t *aSound);

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

#pragma mark - Sound effects

SoundEffect_t *sfx_load(const char *aFilename)
{
#define _CHECK_ERR(msg...) if(status != noErr) { \
	debug_log(msg); \
	return NULL; \
}

	SoundEffect_t *out = obj_create_autoreleased(&Class_SoundEffect);

	CFURLRef url = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, (UInt8*)aFilename, strlen(aFilename), false);

	ExtAudioFileRef AFID;
	OSStatus status = ExtAudioFileOpenURL(url, &AFID);
	CFRelease(url);
	_CHECK_ERR("Could not load %s", aFilename)

	AudioStreamBasicDescription inputFormat;
	UInt32 propSize = sizeof(inputFormat);
	status = ExtAudioFileGetProperty(AFID, kExtAudioFileProperty_FileDataFormat, &propSize, &inputFormat);
	_CHECK_ERR("Error getting file format");
    assert(inputFormat.mChannelsPerFrame <= 2);
	out->channels = inputFormat.mChannelsPerFrame;
	out->rate = inputFormat.mSampleRate;

    // Configure the format that the input data is converted to (Linear PCM)
    AudioStreamBasicDescription outputFormat;
    outputFormat.mSampleRate = inputFormat.mSampleRate;
	outputFormat.mChannelsPerFrame = inputFormat.mChannelsPerFrame;
    
	outputFormat.mFormatID = kAudioFormatLinearPCM;
	outputFormat.mBytesPerPacket = 2 * inputFormat.mChannelsPerFrame;
	outputFormat.mFramesPerPacket = 1;
	outputFormat.mBytesPerFrame = 2 * inputFormat.mChannelsPerFrame;
	outputFormat.mBitsPerChannel = 16;
	outputFormat.mFormatFlags = kAudioFormatFlagsNativeEndian | kAudioFormatFlagIsPacked | kAudioFormatFlagIsSignedInteger;

    status = ExtAudioFileSetProperty(AFID, kExtAudioFileProperty_ClientDataFormat, sizeof(outputFormat), &outputFormat);
    _CHECK_ERR("Error setting internal audio format");

    SInt64 dataLengthInFrames;
    propSize = sizeof(dataLengthInFrames);
    status = ExtAudioFileGetProperty(AFID, kExtAudioFileProperty_FileLengthFrames, &propSize, &dataLengthInFrames);
	_CHECK_ERR("Error getting audio data length");
    

    UInt32 dataSize = dataLengthInFrames*outputFormat.mBytesPerFrame;
	void *data = malloc(dataSize);
	assert(data);
    memset(data, 0, dataSize);
    
    AudioBufferList dataBuffer;
    dataBuffer.mNumberBuffers = 1;
    dataBuffer.mBuffers[0].mDataByteSize = dataSize;
    dataBuffer.mBuffers[0].mNumberChannels = outputFormat.mChannelsPerFrame;
    dataBuffer.mBuffers[0].mData = data;
    status = ExtAudioFileRead(AFID, (UInt32*)&dataLengthInFrames, &dataBuffer);
	_CHECK_ERR("Error decoding audio file data");
	
	out->format = out->channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
	alGenBuffers(1, &out->buffer);
	alGenSources(1, &out->source);
	_checkForOpenAlError();
	alBufferData(out->buffer, out->format, data, dataSize, out->rate);
	_checkForOpenAlError();

	alSourcei(out->source, AL_BUFFER,  out->buffer);
	sfx_setLocation(out, GLMVec3_zero);
	sfx_setLooping(out, false);
	sfx_setPitch(out, 1.0f);
	alSource3f(out->source, AL_VELOCITY,        0.0, 0.0, 0.0);
	alSource3f(out->source, AL_DIRECTION,       0.0, 0.0, 0.0);
	alSourcef (out->source, AL_ROLLOFF_FACTOR,  0.0          );
	alSourcei (out->source, AL_SOURCE_RELATIVE, AL_TRUE      );

	return out;
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
    assert(aFilename);
    NSString *path = [NSString stringWithUTF8String:aFilename];
    NSURL *fileURL = [NSURL fileURLWithPath:path isDirectory:NO];

    NSError *err = nil;
    AVAudioPlayer *player = [[AVAudioPlayer alloc] initWithContentsOfURL:fileURL error:&err];
	if(err) NSLog(@"%@", err);

    if(!player) return NULL;
    
    BackgroundMusic_t *out = obj_create_autoreleased(&Class_BackgroundMusic);
    out->player = player;
    return out;
}

void bgm_unload(BackgroundMusic_t *aBGM)
{
	if(aBGM->player) {
		bgm_stop(aBGM);
		[(id)aBGM->player release], aBGM->player = NULL;
	}
}

static void bgm_destroy(BackgroundMusic_t *aBGM)
{
	bgm_unload(aBGM);
}

void bgm_play(BackgroundMusic_t *aBGM)
{
    [(AVAudioPlayer *)aBGM->player play];
}
void bgm_stop(BackgroundMusic_t *aBGM)
{
    [(AVAudioPlayer *)aBGM->player stop];
}
bool bgm_isPlaying(BackgroundMusic_t *aBGM)
{
    return [(AVAudioPlayer *)aBGM->player isPlaying];
}
void bgm_seek(BackgroundMusic_t *aBGM, float aSeconds)
{
    [(AVAudioPlayer *)aBGM->player setCurrentTime:aSeconds];
}
void bgm_setVolume(BackgroundMusic_t *aBGM, float aVolume)
{
    [(AVAudioPlayer *)aBGM->player setVolume:aVolume];
}

#pragma mark - Sound manager (OpenAL)

SoundManager_t *soundManager_create()
{
	SoundManager_t *out = obj_create_autoreleased(&Class_SoundManager);
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

