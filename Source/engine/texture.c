#include "texture.h"
#include "png_loader.h"
#include "various.h"

static void texture_destroy(Texture_t *aTexture);

const TextureRect_t kTextureRectEntire = { 0.0f, 0.0f, 1.0f, 1.0f };

Texture_t *texture_loadFromPng(const char *aPath, bool aRepeatHorizontal, bool aRepeatVertical)
{
	Texture_t *out  = obj_create_autoreleased(sizeof(Texture_t), (Obj_destructor_t)&texture_destroy);

	int width, height;
	bool hasAlpha;
	GLubyte *data;
	bool success = png_load(aPath, &width, &height, &hasAlpha, &data);
	if (!success) {
		debug_log("Unable to load png file from %s", aPath);
		return NULL;
	}

	glGenTextures(1, &out->id);
	glBindTexture(GL_TEXTURE_2D, out->id);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, hasAlpha ? 4 : 3, width, height,
	             0, hasAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, aRepeatHorizontal ? GL_REPEAT : GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, aRepeatVertical   ? GL_REPEAT : GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	free(data);
	out->size.w = (float)width;
	out->size.h = (float)height;

	return out;
}

void texture_destroy(Texture_t *aTexture)
{
	glDeleteTextures(1, &aTexture->id);
	aTexture->id = 0;
}

TextureRect_t textureRectangle_createWithPixelCoordinates(Texture_t *aTexture, vec2_t aOrigin, vec2_t aSize)
{
	return textureRectangle_create(aOrigin.x/aTexture->size.w, aOrigin.y/aTexture->size.h,
	                               aSize.w/aTexture->size.w,   aSize.h/aTexture->size.h);
}
TextureRect_t textureRectangle_createWithSizeInPixels(Texture_t *aTexture, vec2_t aSize)
{
	return textureRectangle_createWithPixelCoordinates(aTexture, kVec2_zero, aSize);
}

TextureRect_t textureRectangle_create(float aX, float aY, float aWidth, float aHeight)
{
	TextureRect_t out = { aX, aY, aWidth, aHeight };
	return out;
}
