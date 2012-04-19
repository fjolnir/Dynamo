#if defined(__APPLE__)
	#include <sys/time.h>
	#include <unistd.h>
    #if defined(TARGET_OS_IPHONE) || defined(TARGET_OS_SIMULATOR)
        #include <OpenGLES/ES2/gl.h>
        #include <OpenGLES/ES2/glext.h>
    #else
        #include <OpenGL/gl.h>
        #include <OpenGL/glu.h>
    #endif
#else
	#ifdef WIN32
		#include <windows.h>
		#error "Windows currently not being maintained, you'll have to add GLee, glfw or the like to get opengl function pointers"
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
