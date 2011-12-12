#include "drawutils.h"
#include "various.h"

static Shader_t *_texturedShader = NULL;
static Shader_t *_coloredShader = NULL;
static Renderer_t *_renderer = NULL;
static const vec4_t kColorWhite = { 1.0f, 1.0f, 1.0f, 1.0f };

void draw_init(Renderer_t *aDefaultRenderer)
{
	_renderer = aDefaultRenderer;
	if(!_texturedShader) {
		_texturedShader = shader_loadFromFiles("engine/shaders/textured.vsh", "engine/shaders/textured.fsh");
		_texturedShader->uniforms[kShader_colorUniform] = shader_getUniformLocation(_texturedShader, "u_color");
	}
	if(!_coloredShader) {
		_coloredShader = shader_loadFromFiles("engine/shaders/colored.vsh", "engine/shaders/colored.fsh");
		_coloredShader->attributes[kShader_colorAttribute] = shader_getAttributeLocation(_coloredShader, "a_color");
	}
}

void draw_cleanup()
{
	if(_texturedShader) shader_destroy(_texturedShader);
	if(_coloredShader) shader_destroy(_coloredShader);
}


#pragma mark - Texture drawing

void draw_quad(vec3_t aCenter, vec2_t aSize, Texture_t *aTexture, TextureRect_t aTextureArea, vec4_t aColor, float aAngle, bool aFlipHorizontal, bool aFlipVertical)
{
	GLfloat vertices[4*3] = {
		0.0f,    0.0f,    0.0f,
		0.0f,    aSize.h, 0.0f,
		aSize.w, 0.0f,    0.0f,
		aSize.w, aSize.h, 0.0f
	};

	float maxTexX = aTextureArea.origin.x + aTextureArea.size.w;
	float maxTexY = aTextureArea.origin.y + aTextureArea.size.h;
	GLfloat texCoords[4*2] = {
		aFlipHorizontal ? maxTexX : aTextureArea.origin.x, aFlipVertical ? maxTexY : aTextureArea.origin.y,
		aFlipHorizontal ? maxTexX : aTextureArea.origin.x, aFlipVertical ? aTextureArea.origin.y : maxTexY,
		aFlipHorizontal ? aTextureArea.origin.x  :maxTexX, aFlipVertical ? maxTexY : aTextureArea.origin.y,
		aFlipHorizontal ? aTextureArea.origin.x : maxTexX, aFlipVertical ? aTextureArea.origin.y : maxTexY
	};

	// Translate&rotate the quad into it's target location
	aCenter = vec3_floor(aCenter);
	matrix_stack_push(_renderer->worldMatrixStack);
	matrix_stack_translate(_renderer->worldMatrixStack, aCenter.x, aCenter.y, aCenter.z);
	matrix_stack_rotate(_renderer->worldMatrixStack, aAngle, 0.0f, 0.0f, 1.0f);
	matrix_stack_translate(_renderer->worldMatrixStack, floor(aSize.w/-2.0f), floor(aSize.h/-2.0f), 0.0f);

	shader_makeActive(_texturedShader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, aTexture->id);

	shader_updateMatrices(_texturedShader, _renderer);
	glUniform1i(_texturedShader->uniforms[kShader_colormap0Uniform], 0);
	glUniform4fv(_texturedShader->uniforms[kShader_colorUniform], 1, aColor.f);

	glVertexAttribPointer(_texturedShader->attributes[kShader_positionAttribute], 3, GL_FLOAT, GL_FALSE, 0, vertices);
	glEnableVertexAttribArray(_texturedShader->attributes[kShader_positionAttribute]);
	glVertexAttribPointer(_texturedShader->attributes[kShader_texCoord0Attribute], 2, GL_FLOAT, GL_FALSE, 0, texCoords);
	glEnableVertexAttribArray(_texturedShader->attributes[kShader_texCoord0Attribute]);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	shader_makeInactive(_texturedShader);
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
	int numberOfIndices = 6*aNumberOfTiles;
	vec2_t *vertices = calloc(numberOfVertices, sizeof(vec3_t));
	vec2_t *texCoords = calloc(numberOfVertices, sizeof(vec2_t));
	GLushort *indices = calloc(numberOfIndices, sizeof(GLushort));
	
	int lastIndex = 0;
	TextureRect_t currTexRect;
	for(int i = 0; i < aNumberOfTiles; ++i) {
		currTexRect = texAtlas_getTextureRect(aAtlas, (int)aOffsets[i].x, (int)aOffsets[i].y);

		vertices[(4*i)+0].x = aCenterPoints[i].x - aAtlas->size.x/2.0f; vertices[(4*i)+0].y = aCenterPoints[i].y - aAtlas->size.y/2.0f;
		vertices[(4*i)+1].x = aCenterPoints[i].x - aAtlas->size.x/2.0f; vertices[(4*i)+1].y = aCenterPoints[i].y + aAtlas->size.y/2.0f;
		vertices[(4*i)+2].x = aCenterPoints[i].x + aAtlas->size.x/2.0f; vertices[(4*i)+2].y = aCenterPoints[i].y + aAtlas->size.y/2.0f;
		vertices[(4*i)+3].x = aCenterPoints[i].x + aAtlas->size.x/2.0f; vertices[(4*i)+3].y = aCenterPoints[i].y - aAtlas->size.y/2.0f;

		texCoords[(4*i)+0]   = currTexRect.origin;
		texCoords[(4*i)+1].u = currTexRect.origin.u;                      texCoords[(4*i)+1].v = currTexRect.origin.v + currTexRect.size.v;
		texCoords[(4*i)+2].u = currTexRect.origin.u + currTexRect.size.u; texCoords[(4*i)+2].v = currTexRect.origin.v + currTexRect.size.v;
		texCoords[(4*i)+3].u = currTexRect.origin.u + currTexRect.size.u; texCoords[(4*i)+3].v = currTexRect.origin.v;

		indices[lastIndex++] = (4*i)+0;
		indices[lastIndex++] = (4*i)+1;
		indices[lastIndex++] = (4*i)+2;
		indices[lastIndex++] = (4*i)+0;
		indices[lastIndex++] = (4*i)+2;
		indices[lastIndex++] = (4*i)+3;
	}

	matrix_stack_push(_renderer->worldMatrixStack);

	shader_makeActive(_texturedShader);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, aAtlas->texture->id);

	shader_updateMatrices(_texturedShader, _renderer);
	glUniform1i(_texturedShader->uniforms[kShader_colormap0Uniform], 0);
	vec4_t white = {1.0, 1.0, 1.0, 1.0};
	glUniform4fv(_texturedShader->uniforms[kShader_colorUniform], 1, white.f);

	glVertexAttribPointer(_texturedShader->attributes[kShader_positionAttribute], 2, GL_FLOAT, GL_FALSE, 0, vertices);
	glEnableVertexAttribArray(_texturedShader->attributes[kShader_positionAttribute]);
	glVertexAttribPointer(_texturedShader->attributes[kShader_texCoord0Attribute], 2, GL_FLOAT, GL_FALSE, 0, texCoords);
	glEnableVertexAttribArray(_texturedShader->attributes[kShader_texCoord0Attribute]);

	glDrawElements(GL_TRIANGLES, numberOfIndices, GL_UNSIGNED_SHORT, indices);

	shader_makeInactive(_texturedShader);
	matrix_stack_pop(_renderer->worldMatrixStack);

	glBindTexture(GL_TEXTURE_2D, 0);

	free(vertices);
	free(texCoords);
	free(indices);
}


#pragma mark - Primitive drawing (Debug drawing)

void draw_rect(rect_t aRect, float aAngle, vec4_t aColor, bool aShouldFill)
{
	aRect.size = vec2_floor(aRect.size);
	GLfloat vertices[4*2] = {
		0.0f,      0.0f,
		0.0f,      aRect.s.h,
		aRect.s.w, aRect.s.h,
		aRect.s.w, 0.0f
	};
	vec4_t colors[4] = { aColor, aColor, aColor, aColor };

	// Translate&rotate the rectangle into it's target location
	vec2_t center = { aRect.origin.x + (aRect.size.w/2.0f),  aRect.origin.y + (aRect.size.h/2.0f) };

	matrix_stack_push(_renderer->worldMatrixStack);
	matrix_stack_translate(_renderer->worldMatrixStack, floorf(center.x), floorf(center.y), 0.0);
	matrix_stack_rotate(_renderer->worldMatrixStack, aAngle, 0.0f, 0.0f, 1.0f);
	matrix_stack_translate(_renderer->worldMatrixStack, floorf(aRect.s.w/-2.0f), floorf(aRect.s.h/-2.0f), 0.0f);

	shader_makeActive(_coloredShader);

	shader_updateMatrices(_coloredShader, _renderer);

	glVertexAttribPointer(_coloredShader->attributes[kShader_positionAttribute], 2, GL_FLOAT, GL_FALSE, 0, vertices);
	glEnableVertexAttribArray(_coloredShader->attributes[kShader_positionAttribute]);
	glVertexAttribPointer(_coloredShader->attributes[kShader_colorAttribute], 4, GL_FLOAT, GL_FALSE, 0, colors);
	glEnableVertexAttribArray(_coloredShader->attributes[kShader_colorAttribute]);

	glDrawArrays(aShouldFill ? GL_TRIANGLE_FAN : GL_LINE_LOOP, 0, 4);

	shader_makeInactive(_coloredShader);
	matrix_stack_pop(_renderer->worldMatrixStack);

}

void draw_ellipse(vec2_t aCenter, vec2_t aRadii, int aSubdivisions, float aAngle, vec4_t aColor, bool aShouldFill)
{
	float vertices[aSubdivisions*2] ;
	vec4_t colors[aSubdivisions];
	float twoPi = 2.0f*M_PI;
	int count = 0;
	for(float theta = 0.0f; (twoPi - theta) > 0.001; theta += twoPi/(float)aSubdivisions) {
		colors[count/2] = aColor;
		vertices[count++] = cosf(theta) * aRadii.w;
		vertices[count++] = sinf(theta) * aRadii.h;
	}


	// Translate&rotate the ellipse into it's target location
	matrix_stack_push(_renderer->worldMatrixStack);
	matrix_stack_translate(_renderer->worldMatrixStack, floorf(aCenter.x), floorf(aCenter.y), 0.0);
	matrix_stack_rotate(_renderer->worldMatrixStack, aAngle, 0.0f, 0.0f, 1.0f);

	shader_makeActive(_coloredShader);
	shader_updateMatrices(_coloredShader, _renderer);

	glVertexAttribPointer(_coloredShader->attributes[kShader_positionAttribute], 2, GL_FLOAT, GL_FALSE, 0, vertices);
	glEnableVertexAttribArray(_coloredShader->attributes[kShader_positionAttribute]);
	glVertexAttribPointer(_coloredShader->attributes[kShader_colorAttribute], 4, GL_FLOAT, GL_FALSE, 0, colors);
	glEnableVertexAttribArray(_coloredShader->attributes[kShader_colorAttribute]);

	glDrawArrays(aShouldFill ? GL_TRIANGLE_FAN : GL_LINE_LOOP, 0, aSubdivisions);

	shader_makeInactive(_coloredShader);
	matrix_stack_pop(_renderer->worldMatrixStack);
}

void draw_circle(vec2_t aCenter, float radius, int aSubdivisions, vec4_t aColor, bool aShouldFill)
{
	vec2_t size = { radius*2.0f, radius*2.0f };
	draw_ellipse(aCenter, size, aSubdivisions, 0.0f, aColor, aShouldFill);
}

void draw_polygon(int aNumberOfVertices, vec2_t *aVertices, vec4_t aColor, bool aShouldFill)
{
	vec4_t colors[aNumberOfVertices];
	for(int i = 0; i < aNumberOfVertices; ++i) colors[i] = aColor;

	shader_makeActive(_coloredShader);
	shader_updateMatrices(_coloredShader, _renderer);

	glVertexAttribPointer(_coloredShader->attributes[kShader_positionAttribute], 2, GL_FLOAT, GL_FALSE, 0, aVertices);
	glEnableVertexAttribArray(_coloredShader->attributes[kShader_positionAttribute]);
	glVertexAttribPointer(_coloredShader->attributes[kShader_colorAttribute], 4, GL_FLOAT, GL_FALSE, 0, colors);
	glEnableVertexAttribArray(_coloredShader->attributes[kShader_colorAttribute]);

	glDrawArrays(aShouldFill ? GL_TRIANGLE_FAN : GL_LINE_LOOP, 0, aNumberOfVertices);

	shader_makeInactive(_coloredShader);
}

void draw_lineSeg(vec2_t aPointA, vec2_t aPointB, vec4_t aColor)
{
	vec2_t vertices[2] = { vec2_floor(aPointA), vec2_floor(aPointB) };
	vec4_t colors[2] = { aColor, aColor };

	shader_makeActive(_coloredShader);
	shader_updateMatrices(_coloredShader, _renderer);

	glVertexAttribPointer(_coloredShader->attributes[kShader_positionAttribute], 2, GL_FLOAT, GL_FALSE, 0, vertices);
	glEnableVertexAttribArray(_coloredShader->attributes[kShader_positionAttribute]);
	glVertexAttribPointer(_coloredShader->attributes[kShader_colorAttribute], 4, GL_FLOAT, GL_FALSE, 0, colors);
	glEnableVertexAttribArray(_coloredShader->attributes[kShader_colorAttribute]);

	glDrawArrays(GL_LINE_STRIP, 0, 2);

	shader_makeInactive(_coloredShader);
}
