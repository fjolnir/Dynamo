#if !defined(GL_GLEXT_PROTOTYPES) && defined(WIN32)
#define GL_GLEXT_PROTOTYPES

// GL extensions
PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLMULTITEXCOORD2FPROC glMultiTexCoord2f;
PFNGLMULTITEXCOORD3FPROC glMultiTexCoord3f;

PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM3FPROC glUniform3f;
PFNGLUNIFORM4FPROC glUniform4f;
PFNGLUNIFORM2FVPROC glUniform2fv;
PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;
PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;

PFNGLGENFRAMEBUFFERSEXTPROC glGenFramebuffersEXT;
PFNGLBINDFRAMEBUFFEREXTPROC	glBindFramebufferEXT;
PFNGLDELETEFRAMEBUFFERSEXTPROC glDeleteFramebuffersEXT;
PFNGLFRAMEBUFFERTEXTURE2DEXTPROC glFramebufferTexture2DEXT;
PFNGLGENRENDERBUFFERSEXTPROC glGenRenderbuffersEXT;
PFNGLBINDRENDERBUFFEREXTPROC glBindRenderbufferEXT;
PFNGLRENDERBUFFERSTORAGEEXTPROC glRenderbufferStorageEXT;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC glRenderbufferStorageMultisampleEXT;
PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC glFramebufferRenderbufferEXT;
PFNGLDELETERENDERBUFFERSEXTPROC glDeleteRenderbuffersEXT;
PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC glCheckFramebufferStatusEXT;
PFNGLGENERATEMIPMAPEXTPROC glGenerateMipmapEXT;
PFNGLBLITFRAMEBUFFEREXTPROC glBlitFramebufferEXT;

PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLCREATESHADERPROC glCreateShader;
PFNGLGETOBJECTPARAMETERIVARBPROC glGetObjectParameterivARB;
PFNGLGETINFOLOGARBPROC glGetInfoLogARB;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLDELETEPROGRAMPROC glDeleteProgram;

PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;

PFNGLCLIENTACTIVETEXTUREARBPROC glClientActiveTexture;

static void loadGLExtensions()
{
	glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
	glMultiTexCoord2f = (PFNGLMULTITEXCOORD2FPROC)wglGetProcAddress("glMultiTexCoord2f");
	glMultiTexCoord3f = (PFNGLMULTITEXCOORD3FPROC)wglGetProcAddress("glMultiTexCoord3f");

	glShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
	glCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONARBPROC)wglGetProcAddress("glGetUniformLocation");
	glUniform1i = (PFNGLUNIFORM1IARBPROC)wglGetProcAddress("glUniform1i");
	glUniform1f = (PFNGLUNIFORM1FARBPROC)wglGetProcAddress("glUniform1f");
	glUniform2f = (PFNGLUNIFORM2FARBPROC)wglGetProcAddress("glUniform2f");
	glUniform3f = (PFNGLUNIFORM3FARBPROC)wglGetProcAddress("glUniform3f");
	glUniform4f = (PFNGLUNIFORM4FARBPROC)wglGetProcAddress("glUniform4f");

	glUniform2fv = (PFNGLUNIFORM2FVARBPROC)wglGetProcAddress("glUniform2fv");
	glUniformMatrix3fv = (PFNGLUNIFORMMATRIX3FVPROC)wglGetProcAddress("glUniformMatrix3fv");
	glUniformMatrix4fv = (PFNGLUNIFORMMATRIX4FVPROC)wglGetProcAddress("glUniformMatrix4fv");
 
	glGenFramebuffersEXT = (PFNGLGENFRAMEBUFFERSEXTPROC)wglGetProcAddress("glGenFramebuffersEXT");
	glBindFramebufferEXT = (PFNGLBINDFRAMEBUFFEREXTPROC)wglGetProcAddress("glBindFramebufferEXT");
	glDeleteFramebuffersEXT = (PFNGLDELETEFRAMEBUFFERSEXTPROC)wglGetProcAddress("glDeleteFramebuffersEXT");
	glFramebufferTexture2DEXT = (PFNGLFRAMEBUFFERTEXTURE2DEXTPROC)wglGetProcAddress("glFramebufferTexture2DEXT");
	glGenRenderbuffersEXT = (PFNGLGENRENDERBUFFERSEXTPROC)wglGetProcAddress("glGenRenderbuffersEXT");
	glBindRenderbufferEXT = (PFNGLBINDRENDERBUFFEREXTPROC)wglGetProcAddress("glBindRenderbufferEXT");
	glRenderbufferStorageEXT = (PFNGLRENDERBUFFERSTORAGEEXTPROC)wglGetProcAddress("glRenderbufferStorageEXT");
	glRenderbufferStorageMultisampleEXT = (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)wglGetProcAddress("glRenderbufferStorageMultisampleEXT");
	glFramebufferRenderbufferEXT = (PFNGLFRAMEBUFFERRENDERBUFFEREXTPROC)wglGetProcAddress("glFramebufferRenderbufferEXT");
	glDeleteRenderbuffersEXT = (PFNGLDELETERENDERBUFFERSEXTPROC)wglGetProcAddress("glDeleteRenderbuffersEXT");
	glCheckFramebufferStatusEXT = (PFNGLCHECKFRAMEBUFFERSTATUSEXTPROC)wglGetProcAddress("glCheckFramebufferStatusEXT");
	glGenerateMipmapEXT = (PFNGLGENERATEMIPMAPEXTPROC)wglGetProcAddress("glGenerateMipmapEXT");

	glBlitFramebufferEXT = (PFNGLBLITFRAMEBUFFEREXTPROC)wglGetProcAddress("glBlitFramebufferEXT");

	glCreateProgram = (PFNGLCREATEPROGRAMOBJECTARBPROC)wglGetProcAddress("glCreateProgram");
	glCreateShader = (PFNGLCREATESHADEROBJECTARBPROC)wglGetProcAddress("glCreateShader");
	glGetObjectParameterivARB = (PFNGLGETOBJECTPARAMETERIVARBPROC)wglGetProcAddress("glGetObjectParameterivARB");
	glGetInfoLogARB = (PFNGLGETINFOLOGARBPROC)wglGetProcAddress("glGetInfoLogARB");
	glAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
	glDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
	glUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");
	glDeleteProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");

	glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
	glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
	glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");

	glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC)wglGetProcAddress("glClientActiveTexture");
}

#endif
