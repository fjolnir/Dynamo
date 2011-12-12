#include "shader.h"
#include <stdbool.h>
#include "glutils.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "various.h"

const char *kShader_UniformNames[kShader_MaxUniforms] = {
	"u_worldMatrix",
	"u_projectionMatrix",
	"u_colormap0",
	"u_colormap1",
	"u_colormap2",
	"u_colormap3",
	"u_color",
	"u_light0Pos",
	"u_light1Pos"
};
const char *kShader_AttributeNames[kShader_MaxAttributes] = {
	"a_position",
	"a_texCoord0",
	"a_texCoord1",
	"a_texCoord2",
	"a_texCoord3"
};

static bool _shader_link(GLuint aProgramObject);
static GLuint _shader_compile(const char *aSrc, GLenum aType, bool *aoSucceeded);
static void _readFile(const char *aFilePath, size_t *aoLength, char **aoOutput);

Shader_t *shader_load(const char *aVertSrc, const char *aFragSrc)
{
	Shader_t *out = malloc(sizeof(Shader_t));
	out->activationCallback = NULL;
	out->deactivationCallback = NULL;
	for(int i = 0; i < kShader_MaxUniforms; ++i) out->uniforms[i] = -1;
	for(int i = 0; i < kShader_MaxAttributes; ++i) out->attributes[i] = -1;
	out->program = glCreateProgram();

	bool success;

	GLuint vertexShader = _shader_compile(aVertSrc, GL_VERTEX_SHADER, &success);
	assert(success);

	GLuint fragmentShader = _shader_compile(aFragSrc, GL_FRAGMENT_SHADER, &success);
	assert(success);

	glAttachShader(out->program, vertexShader);
	out->vertexShader = vertexShader;
	glAttachShader(out->program, fragmentShader);
	out->fragmentShader = fragmentShader;

	success = _shader_link(out->program);
	assert(success);

	// Hook up the default uniforms&attributes if available
	out->uniforms[kShader_worldMatrixUniform] = shader_getUniformLocation(out, kShader_UniformNames[kShader_worldMatrixUniform]);
	out->uniforms[kShader_projectionMatrixUniform] = shader_getUniformLocation(out, kShader_UniformNames[kShader_projectionMatrixUniform]);
	out->uniforms[kShader_colormap0Uniform] = shader_getUniformLocation(out, kShader_UniformNames[kShader_colormap0Uniform]);

	out->attributes[kShader_positionAttribute] = shader_getAttributeLocation(out, kShader_AttributeNames[kShader_positionAttribute]);
	out->attributes[kShader_texCoord0Attribute] = shader_getAttributeLocation(out, kShader_AttributeNames[kShader_texCoord0Attribute]);

	return out;
}

Shader_t *shader_loadFromFiles(const char *aVertShaderPath, const char *aFragShaderPath)
{
	char *vertShaderSource = NULL, *fragShaderSource = NULL;
	size_t vertShaderLength, fragShaderLength;
	
	_readFile(aVertShaderPath, &vertShaderLength, &vertShaderSource);
	assert(vertShaderLength > 0 && vertShaderSource != NULL);

	_readFile(aFragShaderPath, &fragShaderLength, &fragShaderSource);
	assert(fragShaderLength > 0 && fragShaderSource != NULL);

	Shader_t *out = shader_load(vertShaderSource, fragShaderSource);

	free(vertShaderSource);
	free(fragShaderSource);

	return out;
}

void shader_destroy(Shader_t *aShader)
{
	glDeleteShader(aShader->vertexShader);
	glDeleteShader(aShader->fragmentShader);
	glDeleteProgram(aShader->program);
	free(aShader);
}

#pragma mark - Usage

void shader_makeActive(Shader_t *aShader)
{
	glUseProgram(aShader->program);
	if(aShader->activationCallback != NULL) aShader->activationCallback(aShader);
}

void shader_makeInactive(Shader_t *aShader)
{
	if(aShader->deactivationCallback != NULL) aShader->deactivationCallback(aShader);
	glUseProgram(0);
}


#pragma mark - Uniform&Attribute handling

GLint shader_getUniformLocation(Shader_t *aShader, const char *aUniformName)
{
	GLint location = glGetUniformLocation(aShader->program, aUniformName);
	if(location == -1)
		debug_log("Uniform lookup error: No such uniform (%s)", aUniformName);
	
	return location;
}

GLint shader_getAttributeLocation(Shader_t *aShader, const char *aAttributeName)
{
	GLint location = glGetAttribLocation(aShader->program, aAttributeName);
	if(location == -1)
		debug_log("Attribute lookup error: No such attribute (%s)", aAttributeName);

	return location;
}

void shader_updateMatrices(Shader_t *aShader, Renderer_t *aRenderer)
{
	glUniformMatrix4fv(aShader->uniforms[kShader_worldMatrixUniform], 1, GL_FALSE,
	                   matrix_stack_get_mat4(aRenderer->worldMatrixStack).f);
	glUniformMatrix4fv(aShader->uniforms[kShader_projectionMatrixUniform], 1, GL_FALSE,
	                   matrix_stack_get_mat4(aRenderer->projectionMatrixStack).f);
}


#pragma mark - Compiling&Linking (Private)

static bool _shader_link(GLuint aProgramObject)
{
	glLinkProgram(aProgramObject);

	GLint logLength;
	glGetProgramiv(aProgramObject, GL_INFO_LOG_LENGTH, &logLength);
	if(logLength > 0) {
		GLchar *log = (GLchar *)malloc(logLength);
		glGetProgramInfoLog(aProgramObject, logLength, &logLength, log);
		debug_log(">> Program link log:\n%s", log);
		free(log);
	}
	
	bool success;
	glGetProgramiv(aProgramObject, GL_LINK_STATUS, (GLint *)&success);
	if(success == false)
		debug_log("Failed to link shader program");
	return success;
}

static GLuint _shader_compile(const char *aSrc, GLenum aType, bool *aoSucceeded)
{
	GLuint shaderObject;

	GLchar **source = malloc(sizeof(GLchar*));
	source[0] = (GLchar*)aSrc;
	if(!source) {
		*aoSucceeded = false;
		free(source);
		return 0;
	}

	shaderObject = glCreateShader(aType);
	glShaderSource(shaderObject, 1, (const GLchar **)source, NULL);
	glCompileShader(shaderObject);

	GLint temp;
	glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &temp);
	if (temp > 0) {
		GLchar *log = (GLchar *)malloc(temp);
		glGetShaderInfoLog(shaderObject, temp, &temp, log);
		debug_log(">> %s shader compile log:\n %s\n", aType == GL_FRAGMENT_SHADER ? "Fragment" : "Vertex", log);
		free(log);
	}

	glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &temp);
	if(temp == GL_FALSE) {
		*aoSucceeded = false;
		debug_log(">> Failed to compile shader:\n%s\n---", aSrc);
	}
	*aoSucceeded = true;
	free(source);

	return shaderObject;
}


#pragma mark - Utilities

static void _readFile(const char *aFilePath, size_t *aoLength, char **aoOutput)
{
	FILE *fd = fopen(aFilePath, "r");
	if(!fd) return;

	fseek(fd, 0, SEEK_END);
	*aoLength = ftell(fd);
	if(!*aoLength) return;
	rewind(fd);

	*aoOutput = calloc(*aoLength+1, sizeof(char));
	fread(*aoOutput, sizeof(char), *aoLength, fd);
}
