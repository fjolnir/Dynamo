#include "drawutils.h"

static Shader_t *_basicShader = NULL;
static Renderer_t *_renderer = NULL;
static const vec4_t kColorWhite = { 1.0f, 1.0f, 1.0f, 1.0f };

void draw_init(Renderer_t *aDefaultRenderer)
{
	_renderer = aDefaultRenderer;
	if(!_basicShader) {
		_basicShader = shader_loadFromFiles("engine/shaders/basic.vsh", "engine/shaders/basic.fsh");
		_basicShader->uniforms[kShader_colorUniform] = shader_getUniformLocation(_basicShader, "u_color");
	}
}

void draw_cleanup()
{
	shader_destroy(_basicShader);
}


#pragma mark -

void draw_quad(vec3_t aCenter, vec2_t aSize, Texture_t *aTexture, TextureRect_t aTextureArea, vec4_t aColor, float aAngle, bool aFlipHorizontal, bool aFlipVertical)
{
	GLfloat vertices[4*3] = {
		0.0f,    0.0f,    0.0f,
		aSize.w, 0.0f,    0.0f,
		0.0f,    aSize.h, 0.0f,
		aSize.w, aSize.h, 0.0f
	};

	float maxTexX = aTextureArea.origin.x + aTextureArea.size.w;
	float maxTexY = aTextureArea.origin.y + aTextureArea.size.h;
	GLfloat texCoords[4*2] = {
		aFlipHorizontal ? maxTexX : aTextureArea.origin.x, aFlipVertical ? maxTexY : aTextureArea.origin.y,
		aFlipHorizontal ? aTextureArea.origin.x  :maxTexX, aFlipVertical ? maxTexY : aTextureArea.origin.y,
		aFlipHorizontal ? maxTexX : aTextureArea.origin.x, aFlipVertical ? aTextureArea.origin.y : maxTexY,
		aFlipHorizontal ? aTextureArea.origin.x : maxTexX, aFlipVertical ? aTextureArea.origin.y : maxTexY
	};

	// Translate&rotate the quad into it's target location
	matrix_stack_push(_renderer->worldMatrixStack);
	matrix_stack_translate(_renderer->worldMatrixStack, floorf(aCenter.x), floorf(aCenter.y), floorf(aCenter.z));
	matrix_stack_rotate(_renderer->worldMatrixStack, aAngle, 0.0f, 0.0f, 1.0f);
	matrix_stack_translate(_renderer->worldMatrixStack, aSize.w/-2.0f, aSize.h/-2.0f, 0.0f);

	shader_makeActive(_basicShader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, aTexture->id);

	shader_updateMatrices(_basicShader, _renderer);
	glUniform1i(_basicShader->uniforms[kShader_colormap0Uniform], 0);
	glUniform4fv(_basicShader->uniforms[kShader_colorUniform], 1, aColor.f);

	glVertexAttribPointer(_basicShader->attributes[kShader_positionAttribute], 3, GL_FLOAT, GL_FALSE, 0, vertices);
	glEnableVertexAttribArray(_basicShader->attributes[kShader_positionAttribute]);
	glVertexAttribPointer(_basicShader->attributes[kShader_texCoord0Attribute], 2, GL_FLOAT, GL_FALSE, 0, texCoords);
	glEnableVertexAttribArray(_basicShader->attributes[kShader_texCoord0Attribute]);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	shader_makeInactive(_basicShader);
	matrix_stack_pop(_renderer->worldMatrixStack);

	glBindTexture(GL_TEXTURE_2D, 0);
}

void draw_texturePortion(vec3_t aCenter, Texture_t *aTexture, TextureRect_t aTextureArea, float aScale, float aAngle, bool aFlipHorizontal, bool aFlipVertical)
{
	vec2_t quadSize = vec2_mul(aTexture->size, aTextureArea.size);
	quadSize = vec2_scalarMul(quadSize, aScale);
	draw_quad(aCenter, quadSize, aTexture, aTextureArea, kColorWhite, aAngle, aFlipHorizontal, aFlipVertical);
}

void draw_texture(vec3_t aCenter, Texture_t *aTexture, float aScale, float aAngle, bool aFlipHorizontal, bool aFlipVertical)
{
	draw_texturePortion(aCenter, aTexture, kTextureRectEntire, aScale, aAngle, aFlipHorizontal, aFlipVertical);
}

void draw_textureAtlas(TextureAtlas_t *aAtlas, int aNumberOfTiles, vec2_t *aOffsets, vec2_t *aCenterPoints)
{
	int numberOfVertices = 4*aNumberOfTiles;
	vec3_t *vertices = calloc(numberOfVertices, sizeof(vec3_t));
	vec2_t *texCoords = calloc(numberOfVertices, sizeof(vec2_t));

	TextureRect_t currTexRect;
	for(int i = 0; i < numberOfVertices; i += 4) {
		vertices[i+0].x = aCenterPoints[i].x - aAtlas->size.x/2.0f; vertices[i+0].y = aCenterPoints[i].y - aAtlas->size.y/2.0f;
		vertices[i+1].x = aCenterPoints[i].x + aAtlas->size.x/2.0f; vertices[i+1].y = aCenterPoints[i].y - aAtlas->size.y/2.0f;
		vertices[i+2].x = aCenterPoints[i].x - aAtlas->size.x/2.0f; vertices[i+2].y = aCenterPoints[i].y + aAtlas->size.y/2.0f;
		vertices[i+3].x = aCenterPoints[i].x + aAtlas->size.x/2.0f; vertices[i+3].y = aCenterPoints[i].y + aAtlas->size.y/2.0f;

		currTexRect = texAtlas_getTextureRect(aAtlas, (int)aOffsets[i].x, (int)aOffsets[i].y);
		texCoords[i+0]   = currTexRect.origin;
		texCoords[i+1].u = currTexRect.origin.u + currTexRect.size.u; texCoords[i+1].v = currTexRect.origin.v;
		texCoords[i+2].u = currTexRect.origin.u;                      texCoords[i+2].v = currTexRect.origin.v + currTexRect.size.v;
		texCoords[i+3].u = currTexRect.origin.u + currTexRect.size.u; texCoords[i+3].v = currTexRect.origin.v + currTexRect.size.v;
	}

	matrix_stack_push(_renderer->worldMatrixStack);

	shader_makeActive(_basicShader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, aAtlas->texture->id);

	shader_updateMatrices(_basicShader, _renderer);
	glUniform1i(_basicShader->uniforms[kShader_colormap0Uniform], 0);
	vec4_t white = {1.0};
	glUniform4fv(_basicShader->uniforms[kShader_colorUniform], 1, white.f);

	glVertexAttribPointer(_basicShader->attributes[kShader_positionAttribute], 3, GL_FLOAT, GL_FALSE, 0, vertices);
	glEnableVertexAttribArray(_basicShader->attributes[kShader_positionAttribute]);
	glVertexAttribPointer(_basicShader->attributes[kShader_texCoord0Attribute], 2, GL_FLOAT, GL_FALSE, 0, texCoords);
	glEnableVertexAttribArray(_basicShader->attributes[kShader_texCoord0Attribute]);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, numberOfVertices);

	shader_makeInactive(_basicShader);
	matrix_stack_pop(_renderer->worldMatrixStack);

	glBindTexture(GL_TEXTURE_2D, 0);
	free(vertices);
	free(texCoords);
}
