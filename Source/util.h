#ifndef _UTIL_H_
#define _UTIL_H_

#define ANDROID_APP_IDENTIFIER "jp.panasonic.GeminiTest"

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

typedef void (*InsertionCallback_t)(void *aVal);
typedef void (*RemovalCallback_t)(void *aVal);

/*!
	Returns the filesystem path for a given resource.

	@field name The file name of the resource
	@field ext The file extension of the resource
	@field dir The directory of the resource (relative to the game resource directory)
	@field output The output buffer
	@field maxLen The length of the output buffer
*/
extern bool util_pathForResource(const char *name, const char *ext, const char *dir, char *output, int maxLen);


/*!
	Platform definitions
*/
typedef enum {
	kPlatformMac,
	kPlatformIOS,
	kPlatformAndroid,
	kPlatformWindows,
	kPlatformOther
} Platform_t;

#if defined(__APPLE__)
	#if TARGET_OS_EMBEDDED || TARGET_IPHONE_SIMULATOR
		#define DYNAMO_PLATFORM kPlatformIOS;
	#else
		#define DYNAMO_PLATFORM kPlatformMac;
	#endif
#elif defined(ANDROID)
	#define DYNAMO_PLATFORM kPlatformAndroid;
#elif defined(WIN32)
	#define DYNAMO_PLATFORM kPlatformWindows;
#else
	#define DYNAMO_PLATFORM kPlatformOther;
#endif

/*!
	Returns the platform that the application was compiled for.
*/
extern Platform_t util_platform(void);

#ifdef MAX
#undef MAX
#endif
/*!
	Takes 2 arguments and returns the greater one.
*/
#define MAX(x,y) ( ((x)>(y)) ? (x) : (y) )

#ifdef MIN
#undef MIN
#endif
/*!
	Takes 2 arguments and returns the smaller one.
*/
#define MIN(x,y) ( ((x)<(y)) ? (x) : (y) )

/*!
	Clamps a value between a minimum and a maximum.
*/
#define CLAMP(x, min, max) MAX(min, MIN(x, max))


#pragma mark - Debug logging

#ifdef DYNAMO_DEBUG
	#ifdef ANDROID
	    #include <android/log.h>
	    #define debug_log(fmt, ...) __android_log_print(ANDROID_LOG_DEBUG, "Dynamo", "%s:%u (%s): " fmt "\n", __FILE__, __LINE__, __func__, ## __VA_ARGS__)
		#define debug_log_min(fmt, ...)  __android_log_print(ANDROID_LOG_DEBUG, "Dynamo", fmt "\n", ## __VA_ARGS__)
	#else
	    #define debug_log(fmt, ...) fprintf(stderr, "%s:%u (%s): " fmt "\n", __FILE__, __LINE__, __func__, ## __VA_ARGS__)
		#define debug_log_min(fmt, ...)  fprintf(stderr, fmt "\n", ## __VA_ARGS__)
	#endif
#else
	#define debug_log(fmt, ...)
#endif

// For FFI
extern void _debug_log(const char *str);

/*!
	Reads a file into the passed buffer.
	You are responsible for freeing the output.
*/
void util_readFile(const char *aFilePath, size_t *aoLength, char **aoOutput);

#endif
