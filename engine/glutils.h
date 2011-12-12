#include "glee/glee.h"

#if defined(__APPLE__)
	#include <sys/time.h>
	#include <unistd.h>
#else
	#ifdef WIN32
		#include <windows.h>
    #endif
#endif

#ifdef DEBUG
#define glError() { \
	GLenum err = glGetError(); \
	while (err != GL_NO_ERROR) { \
		debug_log("glError(0x%04x): %s caught", err, (char *)gluErrorString(err)); \
		err = glGetError(); \
	} \
}
#else
#define glError()
#endif
