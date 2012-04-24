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
#define glError()\
{\
for ( GLenum Error = glGetError( ); ( GL_NO_ERROR != Error ); Error = glGetError( ) )\
{\
switch ( Error )\
{\
case GL_INVALID_ENUM:      printf( "\n%s\n\n", "GL_INVALID_ENUM"      ); assert( 0 ); break;\
case GL_INVALID_VALUE:     printf( "\n%s\n\n", "GL_INVALID_VALUE"     ); assert( 0 ); break;\
case GL_INVALID_OPERATION: printf( "\n%s\n\n", "GL_INVALID_OPERATION" ); assert( 0 ); break;\
case GL_OUT_OF_MEMORY:     printf( "\n%s\n\n", "GL_OUT_OF_MEMORY"     ); assert( 0 ); break;\
default:                                                                              break;\
}\
}\
}

#else
#define glError()
#endif
