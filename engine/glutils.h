#if defined(__APPLE__)
	#include <sys/time.h>
	#include <unistd.h>
	#include <OpenGL/gl.h>
	#include <Opengl/glext.h>
	#include <OpenGL/glu.h>
	#include <GLUT/glut.h>
#else
	#define _CRT_SECURE_NO_DEPRECATE
	#define _WIN32_LEAN_AND_MEAN

	#ifdef WIN32
		#include <windows.h>
		#include "engine/windows/gldefs.h"
	#endif
	#include <GL/gl.h>
	#include <GL/glext.h>
	#include <GL/glu.h>
	#include <GL/glut.h>
#endif

#define glError() { \
	GLenum err = glGetError(); \
	while (err != GL_NO_ERROR) { \
		printf( "glError(0x%04x): %s caught at %s:%u\n", err,(char *)gluErrorString(err), __FILE__, __LINE__); \
		err = glGetError(); \
	} \
}
