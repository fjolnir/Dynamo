#include "background.h"
#include "shader.h"
#include "util.h"

static void _background_draw(Renderer_t *aRenderer, Background_t *aBackground, GLMFloat aTimeSinceLastFrame, GLMFloat aInterpolation);
static void background_destroy(Background_t *aBackground);
static void background_destroyLayer(BackgroundLayer_t *aLayer);

Class_t Class_Background = {
	"Background",
	sizeof(Background_t),
	(Obj_destructor_t)&background_destroy
};
Class_t Class_BackgroundLayer = {
	"BackgroundLayer",
	sizeof(BackgroundLayer_t),
	(Obj_destructor_t)&background_destroyLayer
};



// Shared amongst all background objects
static Shader_t *_backgroundShader;

enum {
	kBackground_offsetUniform = kShader_colorUniform, // Begin our additional uniform indices after the last color map
	kBackground_sizeUniform,
	kBackground_layer0DepthUniform,
	kBackground_layer1DepthUniform,
	kBackground_layer2DepthUniform,
	kBackground_layer3DepthUniform
};

Background_t *background_create()
{
	Background_t *out = obj_create_autoreleased(&Class_Background);
	out->offset = GLMVec2_zero;
	out->displayCallback = (RenderableDisplayCallback_t)&_background_draw;

	if(!_backgroundShader) {
		const int maxLen = 1024;
		char vshPath[maxLen], fshPath[maxLen];
		assert(util_pathForResource("background", "vsh", "Shaders" vshPath, maxLen));
		assert(util_pathForResource("background", "fsh", "Shaders" fshPath, maxLen));
		_backgroundShader = obj_retain(shader_loadFromFiles(vshPath, fshPath));
		_backgroundShader->uniforms[kBackground_offsetUniform] = shader_getUniformLocation(_backgroundShader, "u_backgroundOffset");
		_backgroundShader->uniforms[kBackground_layer0DepthUniform] = shader_getUniformLocation(_backgroundShader, "u_layer0Depth");
		_backgroundShader->uniforms[kBackground_layer1DepthUniform] = shader_getUniformLocation(_backgroundShader, "u_layer1Depth");
		_backgroundShader->uniforms[kBackground_layer2DepthUniform] = shader_getUniformLocation(_backgroundShader, "u_layer2Depth");
		_backgroundShader->uniforms[kBackground_layer3DepthUniform] = shader_getUniformLocation(_backgroundShader, "u_layer3Depth");
		_backgroundShader->uniforms[kShader_colormap1Uniform] = shader_getUniformLocation(_backgroundShader, "u_colormap1");
		_backgroundShader->uniforms[kShader_colormap2Uniform] = shader_getUniformLocation(_backgroundShader, "u_colormap2");
		_backgroundShader->uniforms[kShader_colormap3Uniform] = shader_getUniformLocation(_backgroundShader, "u_colormap3");
	}

	return out;
}

static void background_destroy(Background_t *aBackground)
{
	for(int i = 0; i < kBackground_maxLayers; ++i) {
		obj_release(aBackground->layers[i]);
		aBackground->layers[i] = NULL;
	}
}

void background_setLayer(Background_t *aBackground, unsigned int aIndex, BackgroundLayer_t *aLayer) {
	assert(aIndex < kBackground_maxLayers);
	aBackground->layers[aIndex] = obj_retain(aLayer);
}

BackgroundLayer_t *background_createLayer(Texture_t *aTexture, float aDepth)
{
	BackgroundLayer_t *out = obj_create_autoreleased(&Class_BackgroundLayer);
	out->texture = obj_retain(aTexture);
	out->depth = aDepth;

	return out;
}

static void background_destroyLayer(BackgroundLayer_t *aLayer)
{
	obj_release(aLayer->texture);
	aLayer->texture = NULL;
}

static void _background_draw(Renderer_t *aRenderer, Background_t *aBackground, GLMFloat aTimeSinceLastFrame, GLMFloat aInterpolation)
{
	if(!aBackground->layers[0])
		return;

	GLfloat vertices[4*2] = {
		-1.0f, -1.0f,
		 1.0f, -1.0f,
		-1.0f, 1.0f,
		 1.0f, 1.0f
	};
	vec2_t viewportSize = aRenderer->viewportSize;
	vec2_t textureSize = aBackground->layers[0]->texture->size;
	GLfloat texCoords[4*2] = {
		0.0f,                         0.0f,
		viewportSize.w/textureSize.w, 0.0f,
		0.0f,                         viewportSize.h/textureSize.h,
		viewportSize.w/textureSize.w, viewportSize.h/textureSize.h
	};

	shader_makeActive(_backgroundShader);
	
	vec2_t uvOffset = vec2_div(aBackground->offset, textureSize);
	glUniform2f(_backgroundShader->uniforms[kBackground_offsetUniform], uvOffset.x, uvOffset.y);
	glUniform2f(_backgroundShader->uniforms[kBackground_sizeUniform], textureSize.w, textureSize.h);

	if(aBackground->layers[0]) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, aBackground->layers[0]->texture->id);
		glUniform1i(_backgroundShader->uniforms[kShader_colormap0Uniform], 0);
	}
	if(aBackground->layers[1]) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, aBackground->layers[1]->texture->id);
		glUniform1i(_backgroundShader->uniforms[kShader_colormap1Uniform], 1);
	}
	if(aBackground->layers[2]) {
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, aBackground->layers[2]->texture->id);
		glUniform1i(_backgroundShader->uniforms[kShader_colormap2Uniform], 2);
	}
	if(aBackground->layers[3]) {
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, aBackground->layers[3]->texture->id);
		glUniform1i(_backgroundShader->uniforms[kShader_colormap3Uniform], 3);
	}

	glUniform1f(_backgroundShader->uniforms[kBackground_layer0DepthUniform],
	            aBackground->layers[0] != NULL ? aBackground->layers[0]->depth : -1.0f);
	glUniform1f(_backgroundShader->uniforms[kBackground_layer1DepthUniform],
	            aBackground->layers[1] != NULL ? aBackground->layers[1]->depth : -1.0f);
	glUniform1f(_backgroundShader->uniforms[kBackground_layer2DepthUniform],
	            aBackground->layers[2] != NULL ? aBackground->layers[2]->depth : -1.0f);
	glUniform1f(_backgroundShader->uniforms[kBackground_layer3DepthUniform],
	            aBackground->layers[3] != NULL ? aBackground->layers[3]->depth : -1.0f);

	glVertexAttribPointer(_backgroundShader->attributes[kShader_positionAttribute], 2, GL_FLOAT, GL_FALSE, 0, vertices);
	glEnableVertexAttribArray(_backgroundShader->attributes[kShader_positionAttribute]);
	glVertexAttribPointer(_backgroundShader->attributes[kShader_texCoord0Attribute], 2, GL_FLOAT, GL_FALSE, 0, texCoords);
	glEnableVertexAttribArray(_backgroundShader->attributes[kShader_texCoord0Attribute]);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	shader_makeInactive(_backgroundShader);
}
