#include "texture_atlas.h"

TextureAtlas_t *texAtlas_create(Texture_t *aTexture, vec2_t aOrigin, vec2_t aSize)
{
	TextureAtlas_t *out = malloc(sizeof(TextureAtlas_t));
	out->texture = aTexture;
	out->origin = aOrigin;
	out->size = aSize;
	out->margin = kVec2_zero;

	return out;
}
void texAtlas_destroy(TextureAtlas_t *aAtlas, bool aShouldDestroyTexture)
{
	if(aShouldDestroyTexture) texture_destroy(aAtlas->texture);
	free(aAtlas);
}

TextureRect_t texAtlas_getTextureRect(TextureAtlas_t *aAtlas, int aX, int aY)
{
	vec2_t offsets = { (float)aX, (float)aY };
	vec2_t accumulatedMargins = vec2_mul(offsets, aAtlas->margin);
	vec2_t origin = vec2_add(vec2_add(vec2_mul(aAtlas->size, offsets), aAtlas->origin), accumulatedMargins);

	return textureRectangle_createWithPixelCoordinates(aAtlas->texture, origin, aAtlas->size);
}
