#include "glutils.h"
#include "GLMath/GLMath.h"

#ifndef _TEXTURE_H_
#define _TEXTURE_H_

typedef struct _Texture {
	GLuint id;
	vec2_t size;
} Texture_t;

// A convenience structure to specify areas to sample from a texture (in UV coordinates)
typedef union _TextureRect {
	vec4_t v;
	struct {
		vec2_t origin;
		vec2_t size;
	};
	struct {
		float u, v;
		float w, h;
	};
} TextureRect_t;

extern const TextureRect_t kTextureRectEntire;

extern Texture_t *texture_loadFromPng(const char *aPath, bool aRepeatHorizontal, bool aRepeatVertical);
extern void texture_destroy(Texture_t *aTexture);
// Generates a UV texture rectangle from pixel coordinates
extern TextureRect_t textureRectangle_createWithPixelCoordinates(Texture_t *aTexture, vec2_t aOrigin, vec2_t aSize);
extern TextureRect_t textureRectangle_createWithSizeInPixels(Texture_t *aTexture, vec2_t aSize);
extern TextureRect_t textureRectangle_create(float aX, float aY, float aWidth, float aHeight);
#endif
