#include "object.h"
#include "glutils.h"
#include "renderer.h"
#include "GLMath/GLMath.h"

#ifndef _TEXTURE_H_
#define _TEXTURE_H_

/*!
	A texture

	@field location The location to use if the texture is drawn directly as a renderable
	@field id The OpenGL texture id
	@field size The image size in pixels
*/
typedef struct _Texture {
	OBJ_GUTS
    RENDERABLE_GUTS
	vec3_t location;
	GLuint id;
	vec2_t size;
	// We need to inset the texture rectangle ever so slightly in order
	// to prevent bleeding when using texture atlases
	vec2_t pxAlignInset;
} Texture_t;
extern Class_t Class_Texture;

/*!
	A  structure to specify areas to sample from a texture (in UV coordinates)
*/
typedef union _TextureRect {
	vec4_t vec;
	float *f;
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

/*!
	Loads a texture from a PNG file.
*/
extern Texture_t *texture_loadFromPng(const char *aPath, bool aRepeatHorizontal, bool aRepeatVertical);
/*!
	Generates a UV texture rectangle from pixel coordinates.
*/
extern TextureRect_t textureRectangle_createWithPixelCoordinates(Texture_t *aTexture, vec2_t aOrigin, vec2_t aSize);
/*!
	Generates a UV texture rectangle from, a pixel size, located at (0,0).
*/
extern TextureRect_t textureRectangle_createWithSizeInPixels(Texture_t *aTexture, vec2_t aSize);
/*!
	Creates a texture rectangle from UV (0.0-1.0) texture coordinates.
*/
extern TextureRect_t textureRectangle_create(float aX, float aY, float aWidth, float aHeight);
#endif
