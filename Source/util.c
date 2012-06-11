#include "util.h"
#include <sys/stat.h>
#include <string.h>
#if defined(__APPLE__)
	#include <CoreFoundation/CoreFoundation.h>
#endif

typedef signed char BOOL;

bool util_pathForResource(const char *name, const char *ext, const char *dir, char *output, int maxLen)
{
    dynamo_assert(output != NULL && maxLen > 0, "Invalid output buffer");
    dynamo_assert(name != NULL || dir != NULL, "You must provide either a name or directory to look up");

    output[0] = '\0';
    #define APPEND(strToAppend) { \
        off_t __ofs = strlen(output); \
        if(strToAppend) strncpy(output+__ofs, strToAppend, maxLen - __ofs); \
    }
    
    #if defined(ANDROID)
        APPEND("/data/data/")
        APPEND(ANDROID_APP_IDENTIFIER)
		APPEND("/files/GameResources/")
    #elif defined(__APPLE__)
        CFURLRef resUrl = CFBundleCopyResourcesDirectoryURL(CFBundleGetMainBundle());
        CFURLGetFileSystemRepresentation(resUrl, true, (UInt8*)output, maxLen);
        APPEND("/")
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
}

extern Platform_t util_platform(void)
{
	return DYNAMO_PLATFORM;
}

void _dynamo_log(const char *str)
{
	dynamo_log_min("%s", str);
}

// TODO: Add graceful error handling (Was failing with a mysterious sigsegv on missing files on android)
void util_readFile(const char *aFilePath, size_t *aoLength, char **aoOutput)
{
    dynamo_assert(aoOutput != NULL, "Output buffer required");
    
	FILE *fd = fopen(aFilePath, "r");
	if(!fd) {
        if(aoLength) *aoLength = 0;
        return;
    }
    
	fseek(fd, 0, SEEK_END);
    size_t len = ftell(fd);
    if(aoLength)
        *aoLength = len;
	if(len <= 0) return;
	rewind(fd);
    
	*aoOutput = calloc(len+1, sizeof(char));
	fread(*aoOutput, sizeof(char), len, fd);
}
