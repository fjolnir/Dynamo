#include <stdio.h>
#include <time.h>

#ifndef _VARIOUS_H_
#define _VARIOUS_H_

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

#ifdef TWODEEDENG_DEBUG
	#define debug_log(fmt, ...) fprintf(stderr, "%s:%u (%s): " fmt "\n", __FILE__, __LINE__, __func__, ## __VA_ARGS__)
#else
	#define debug_log(fmt, ...)
#endif


#pragma mark - OpenGL debugging

#ifdef TWODEEDENG_DEBUG

#include <OpenGL/gl.h>

#include <stdlib.h>
#include <assert.h>

#define CHECK_GL_ERR()\
{\
	for ( GLenum Error = glGetError( ); ( GL_NO_ERROR != Error ); Error = glGetError( ) )\
	{\
		switch ( Error )\
		{\
			case GL_INVALID_ENUM:      printf( "\n%s\n\n", "GL_INVALID_ENUM"      ); assert( 0 ); break;\
			case GL_INVALID_VALUE:     printf( "\n%s\n\n", "GL_INVALID_VALUE"     ); assert( 0 ); break;\
			case GL_INVALID_OPERATION: printf( "\n%s\n\n", "GL_INVALID_OPERATION" ); assert( 0 ); break;\
			case GL_STACK_OVERFLOW:    printf( "\n%s\n\n", "GL_STACK_OVERFLOW"    ); assert( 0 ); break;\
			case GL_STACK_UNDERFLOW:   printf( "\n%s\n\n", "GL_STACK_UNDERFLOW"   ); assert( 0 ); break;\
			case GL_OUT_OF_MEMORY:     printf( "\n%s\n\n", "GL_OUT_OF_MEMORY"     ); assert( 0 ); break;\
			default:                                                                              break;\
		}\
	}\
}

#define CHECK_FB_STATUS( )\
{\
	switch ( glCheckFramebufferStatus( GL_FRAMEBUFFER ) )\
	{\
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:         printf( "\n%s\n\n", "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"         ); assert( 0 ); break;\
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: printf( "\n%s\n\n", "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" ); assert( 0 ); break;\
	case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:        printf( "\n%s\n\n", "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"        ); assert( 0 ); break;\
	case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:        printf( "\n%s\n\n", "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"        ); assert( 0 ); break;\
	case GL_FRAMEBUFFER_UNSUPPORTED:                   printf( "\n%s\n\n", "GL_FRAMEBUFFER_UNSUPPORTED"                   ); assert( 0 ); break;\
	case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:        printf( "\n%s\n\n", "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE"        ); assert( 0 ); break;\
	case GL_FRAMEBUFFER_UNDEFINED:                     printf( "\n%s\n\n", "GL_FRAMEBUFFER_UNDEFINED"                     ); assert( 0 ); break;\
	default:                                                                                                                              break;\
	}\
}

#define GET_SHADER_INFOLOG( Shader, Source )\
{\
	GLint   Status, Count;\
	GLchar *Error;\
	\
	glGetShaderiv( Shader, GL_COMPILE_STATUS, &Status );\
	if ( !Status )\
	{\
		glGetShaderiv( Shader, GL_INFO_LOG_LENGTH, &Count );\
		if ( Count > 0 )\
		{\
			glGetShaderInfoLog( Shader, Count, NULL, ( Error = calloc( 1, Count ) ) );\
			printf( "Shaderlog: %s\n\n%s\n", Source, Error );\
			free( Error );\
			\
			assert( 0 );\
		}\
	}\
}

#define CLEAR_GL_ERRORS() while(glGetError() != GL_NO_ERROR)

#else
#define CHECK_GL_ERR( )
#define CHECK_FB_STATUS( )
#define GET_SHADER_INFOLOG( Shader, Source )
#define CLEAR_GL_ERRORS()

#endif

#endif
