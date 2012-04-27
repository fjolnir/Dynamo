// Splits a texture into multiple evenly sized chunks (Not in memory)
// relative to a starting point.

// Usage example: multiple atlases share the same texture, but use a different origin&size
//   thereby allowing multiple differently sized objects to be drawn using a single texture in a single draw call

#include "object.h"
#include "texture.h"

#ifndef _ATLAS_H_
#define _ATLAS_H_

extern Class_t Class_TextureAtlas;
typedef struct _TextureAtlas {
	OBJ_GUTS
	vec2_t origin; // The point to consider as (0,0)
	vec2_t size; // The size of each subtexture
	vec2_t margin; // The gap between frames in the texture
	Texture_t *texture;
} TextureAtlas_t;

extern TextureAtlas_t *texAtlas_create(Texture_t *aTexture, vec2_t aOrigin, vec2_t aSize);
// Gets the sampling rectangle for the subtexture at a given offset
extern TextureRect_t texAtlas_getTextureRect(TextureAtlas_t *aAtlas, int x, int y);
#endif
