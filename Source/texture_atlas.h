/*!
    @header Texture Atlas
    @abstract
    @discussion Splits a texture into multiple evenly sized chunks (Not in memory) relative to a starting point.

    <b>Usage case for relative starting point:</b> multiple atlases share the same texture, but use a different origin&size
    thereby allowing multiple differently sized objects to be drawn using a single texture in a single draw call
*/

#include "object.h"
#include "texture.h"

#ifndef _ATLAS_H_
#define _ATLAS_H_

/*!
    A texture atlas.

    @field origin The starting point of the atlas within the texture
    @field size The size of each cell in the atlas
    @field margin The gap between cells in the atlas.
    @field texture The sampled texture
*/
typedef struct _TextureAtlas {
    OBJ_GUTS
    vec2_t origin; // The point to consider as (0,0)
    vec2_t size; // The size of each subtexture
    vec2_t margin; // The gap between frames in the texture
    Texture_t *texture;
} TextureAtlas_t;
extern Class_t Class_TextureAtlas;

/*!
    Creates an atlas from a given texture.
*/
extern TextureAtlas_t *texAtlas_create(Texture_t *aTexture, vec2_t aOrigin, vec2_t aSize);
/*!
    Gets the sampling rectangle for the subtexture at a given cell offset.
*/
extern TextureRect_t texAtlas_getTextureRect(TextureAtlas_t *aAtlas, int x, int y);
#endif
