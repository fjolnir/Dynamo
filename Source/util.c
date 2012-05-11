#include "util.h"
#include <sys/stat.h>
#include <string.h>
#if defined(__APPLE__)
	#include <CoreFoundation/CoreFoundation.h>
#endif

BOOL util_pathForResource(const char *name, const char *ext, const char *dir, char *output, int maxLen)
{
    assert(output != NULL && maxLen > 0);

#if defined(__APPLE__) // iOS

    CFBundleRef bundle = CFBundleGetMainBundle();
    CFStringRef cfName = CFStringCreateWithCString(NULL, name, kCFStringEncodingUTF8);
    CFStringRef cfExt = ext ? CFStringCreateWithCString(NULL, ext, kCFStringEncodingUTF8) : NULL;
    CFStringRef cfDir = dir ? CFStringCreateWithCString(NULL, dir, kCFStringEncodingUTF8) : NULL;
    CFURLRef url = CFBundleCopyResourceURL(bundle, cfName, cfExt, cfDir);
    
    if(cfName) CFRelease(cfName);
    if(cfExt) CFRelease(cfExt);
    if(cfDir) CFRelease(cfDir);
    if(!url)
        return false;
    
    BOOL ret = CFURLGetFileSystemRepresentation(url, true, (UInt8*)output, maxLen);
    CFRelease(url);
    return ret;
    
#else
    
    output[0] = '\0';
    #define APPEND(strToAppend) { \
        off_t __ofs = strlen(output); \
        if(strToAppend) strncpy(output+__ofs, strToAppend, maxLen - __ofs); \
    }
    
    #if defined(ANDROID)
        APPEND("/data/data/")
        APPEND(ANDROID_APP_IDENTIFIER)
		APPEND("/files/game_assets/")
    #endif
    APPEND(dir)
	if(dir && dir[strlen(dir)-1] != '/')
		APPEND("/")
    APPEND(name)
    if(ext) {
        APPEND(".")
        APPEND(ext)
    }
    
    struct stat unused;
    return stat(output, &unused) == 0;
    #undef APPEND
    
#endif
}

extern Platform_t util_platform(void)
{
	return DYNAMO_PLATFORM;
}

void _debug_log(const char *str)
{
	debug_log_min("%s", str);
}
