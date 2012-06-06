/*!
	@header Texture
	@abstract
	@discussion Provides texture loading.
*/

#include "object.h"
#include "glutils.h"
#include "renderer.h"
#include "dictionary.h"
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
	Texture packing info

 	A helper for getting the TextureRect of a subtexture packed within a larger
	texture. (Different from an atlas in that the subtextures are neither the same size, or part of an animation)
 
    Packed textures can be generated using JSON export in TexturePacker. (http://texturepacker.com)
 */
typedef struct _TexturePackingInfo {
	OBJ_GUTS
    RENDERABLE_GUTS
	Dictionary_t *subtextures;
} TexturePackingInfo_t;
extern Class_t Class_TexturePackingInfo;

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

/*!
	Creates a texture packing info object from a JSON file.
*/
extern TexturePackingInfo_t *texturePacking_load(const char *aPath);
/*!
	Returns the texture rect for a subtexture matching aTexName.
*/
extern TextureRect_t texturePacking_getRect(TexturePackingInfo_t *aPacking, Texture_t *aSourceTex, const char *aTexName);
/*!
 Returns the origin for a subtexture matching aTexName in pixels.
 */
extern vec2_t texturePacking_getPixelOrigin(TexturePackingInfo_t *aPacking, Texture_t *aSourceTex, const char *aTexName);


#include "texture_atlas.h"
/*!
 Returns an atlas matching aTexName in aSourceTex
*/
extern TextureAtlas_t *texturePacking_getAtlas(TexturePackingInfo_t *aPacking, Texture_t *aSourceTex, const char *aTexName, vec2_t aAtlasSize);

#endif
