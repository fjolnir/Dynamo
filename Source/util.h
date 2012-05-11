#ifndef _UTIL_H_
#define _UTIL_H_

#define ANDROID_APP_IDENTIFIER "jp.panasonic.GeminiTest"

#include <stdio.h>
#include <assert.h>


typedef void (*InsertionCallback_t)(void *aVal);
typedef void (*RemovalCallback_t)(void *aVal);

typedef signed char BOOL;
extern BOOL util_pathForResource(const char *name, const char *ext, const char *dir, char *output, int maxLen);


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

extern Platform_t util_platform(void);

#ifdef MAX
#undef MAX
#endif
#define MAX(x,y) ( ((x)>(y)) ? (x) : (y) )

#ifdef MIN
#undef MIN
#endif
#define MIN(x,y) ( ((x)<(y)) ? (x) : (y) )

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

#endif
