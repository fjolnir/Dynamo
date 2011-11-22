// Shader loading
#include "glutils.h"
#include "renderer.h"

#ifndef __SHADER_H_
#define __SHADER_H_

#define kShader_MaxUniforms 16
#define kShader_MaxAttributes 16

// Default uniform set (Hooked up automatically on creation, if possible)
enum {
	kShader_worldMatrixUniform,
	kShader_projectionMatrixUniform,
	kShader_colormap0Uniform,
	// Indices below this point are not hooked up automatically
	kShader_colormap1Uniform,
	kShader_colormap2Uniform,
	kShader_colormap3Uniform,
	kShader_colorUniform,
	kShader_light0Uniform,
	kShader_light1Uniform
};
// Default attribute set (Hooked up automatically on creation, if possible)
enum {
	kShader_positionAttribute,
	kShader_texCoord0Attribute,
	// Indices below this point are not hooked up automatically
	kShader_texCoord1Attribute,
	kShader_texCoord2Attribute,
	kShader_texCoord3Attribute
};

extern const char *kShader_UniformNames[kShader_MaxUniforms];
extern const char *kShader_AttributeNames[kShader_MaxAttributes];

typedef struct _Shader Shader_t;
struct _Shader {
	GLuint program, vertexShader, fragmentShader;
	GLint uniforms[kShader_MaxUniforms];
	GLint attributes[kShader_MaxAttributes];
	// Used so different shaders can perform different setup/teardown when made (de)active
	void (*activationCallback)(Shader_t *aSelf); // Called after the shader is bound
	void (*deactivationCallback)(Shader_t *aSelf); // Called before the shader is unbound
};

extern Shader_t *shader_load(const char *aVertSrc, const char *aFragSrc);
extern Shader_t *shader_loadFromFiles(const char *aVertShaderPath, const char *aFragShaderPath);
extern void shader_destroy(Shader_t *aShader);

extern void shader_makeActive(Shader_t *aShader);
extern void shader_makeInactive(Shader_t *aShader);

extern GLint shader_getUniformLocation(Shader_t *aShader, const char *aUniformName);
extern GLint shader_getAttributeLocation(Shader_t *aShader, const char *aAttributeName);
// Updates the matrix uniforms using the renderer object passed
extern void shader_updateMatrices(Shader_t *aShader, Renderer_t *aRenderer);

#endif
