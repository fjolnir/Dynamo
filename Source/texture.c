#include "texture.h"
#include "png_loader.h"
#include "util.h"

static void texture_destroy(Texture_t *aTexture);

static Class_t Class_Texture = {
	"Texture",
	sizeof(Texture_t),
	(Obj_destructor_t)&texture_destroy
};

const TextureRect_t kTextureRectEntire = { 0.0f, 0.0f, 1.0f, 1.0f };

Texture_t *texture_loadFromPng(const char *aPath, bool aRepeatHorizontal, bool aRepeatVertical)
{
	Texture_t *out = obj_create_autoreleased(&Class_Texture);

	Png_t *png = png_load(aPath);
	if (!png) {
		debug_log("Unable to load png file from %s", aPath);
		return NULL;
	}

	glGenTextures(1, &out->id);
	glBindTexture(GL_TEXTURE_2D, out->id);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(GL_TEXTURE_2D, 0, png->hasAlpha ? GL_RGBA : GL_RGB, png->width, png->height,
	             0, png->hasAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, png->data);
    glError()
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, aRepeatHorizontal ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, aRepeatVertical   ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	out->size.w = (float)png->width;
	out->size.h = (float)png->height;

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
	return textureRectangle_createWithPixelCoordinates(aTexture, GLMVec2_zero, aSize);
}

TextureRect_t textureRectangle_create(float aX, float aY, float aWidth, float aHeight)
{
	TextureRect_t out = { aX, aY, aWidth, aHeight };
	return out;
}
