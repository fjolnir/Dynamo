#include "texture_atlas.h"
#include "util.h"

static void texAtlas_destroy(TextureAtlas_t *aAtlas);

Class_t Class_TextureAtlas = {
    "TextureAtlas",
    sizeof(TextureAtlas_t),
    (Obj_destructor_t)&texAtlas_destroy
};

TextureAtlas_t *texAtlas_create(Texture_t *aTexture, vec2_t aOrigin, vec2_t aSize)
{
    TextureAtlas_t *out = obj_create_autoreleased(&Class_TextureAtlas);
    out->texture = obj_retain(aTexture);
    out->origin = aOrigin;
    out->size = aSize;
    out->margin = GLMVec2_zero;

    return out;
}
void texAtlas_destroy(TextureAtlas_t *aAtlas)
{
    obj_release(aAtlas->texture);
    aAtlas->texture = NULL;
}

TextureRect_t texAtlas_getTextureRect(TextureAtlas_t *aAtlas, int aX, int aY)
{
    vec2_t offsets = { (float)aX, (float)aY };
    vec2_t accumulatedMargins = vec2_mul(offsets, aAtlas->margin);
    vec2_t origin = vec2_add(vec2_add(vec2_mul(aAtlas->size, offsets), aAtlas->origin), accumulatedMargins);

    return textureRectangle_createWithPixelCoordinates(aAtlas->texture, origin, aAtlas->size);
}
