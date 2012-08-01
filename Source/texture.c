#include "texture.h"
#include "png_loader.h"
#include "util.h"
#include "drawutils.h"
#include "json.h"

static void texture_destroy(Texture_t *aTexture);
static void _texture_draw(Renderer_t *aRenderer, Texture_t *aTexture, GLMFloat aTimeSinceLastFrame, GLMFloat aInterpolation);

Class_t Class_Texture = {
    "Texture",
    sizeof(Texture_t),
    (Obj_destructor_t)&texture_destroy
};

const TextureRect_t kTextureRectEntire = { 0.0f, 0.0f, 1.0f, 1.0f };

static inline bool _isPowerOfTwo(int n)
{
    return (n != 0) && ((n & (n - 1)) == 0);
}

Texture_t *texture_loadFromPng(const char *aPath, bool aRepeatHorizontal, bool aRepeatVertical)
{
    Texture_t *out = obj_create_autoreleased(&Class_Texture);
    out->displayCallback = (RenderableDisplayCallback_t)&_texture_draw;
    
    Png_t *png = png_load(aPath);
    if(!png) {
        dynamo_log("Unable to load png file from %s", aPath);
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
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // Mipmaps can only be generated if the texture size is a power of 2
    if(_isPowerOfTwo(png->width) && _isPowerOfTwo(png->height) && !aRepeatHorizontal && !aRepeatVertical) {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
            dynamo_assert(!( (!_isPowerOfTwo(png->width) || !_isPowerOfTwo(png->height))
                              && (aRepeatHorizontal || aRepeatVertical) ),
                          "Repeating textures must have power of 2 dimensions");
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    glError()
    
    out->size = vec2_create(png->width, png->height);
    out->pxAlignInset = vec2_create(
                                    (1.0f/out->size.w) * 0.5,
                                    (1.0f/out->size.h) * 0.5
                                    );
    
    return out;
}

void texture_destroy(Texture_t *aTexture)
{
    if(aTexture->subtextures)
        obj_release(aTexture->subtextures);
    glDeleteTextures(1, &aTexture->id);
    aTexture->id = 0;
}

TextureRect_t textureRectangle_createWithPixelCoordinates(Texture_t *aTexture, vec2_t aOrigin, vec2_t aSize)
{
    return textureRectangle_create(
                                   aTexture->pxAlignInset.w + aOrigin.x/aTexture->size.w,
                                   aTexture->pxAlignInset.h + aOrigin.y/aTexture->size.h,
                                   aSize.w/aTexture->size.w - aTexture->pxAlignInset.w,
                                   aSize.h/aTexture->size.h - aTexture->pxAlignInset.h);
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

static void _texture_draw(Renderer_t *aRenderer, Texture_t *aTexture, GLMFloat aTimeSinceLastFrame, GLMFloat aInterpolation)
{
    draw_texture(aTexture->location, aTexture, 1.0, 0.0, false, false);
}


extern bool texture_loadPackingInfo(Texture_t *aTexture, const char *aPath)
{
    char *jsonInput;
    util_readFile(aPath, NULL, &jsonInput);
    dynamo_assert(jsonInput != NULL, "No file at %s", aPath);
    Dictionary_t *info = parseJSON(jsonInput);
    free(jsonInput);
    dynamo_assert(info != NULL && dict_get(info, "frames") != NULL, "Could not load texture packing info from %s", aPath);
    
    aTexture->subtextures = obj_retain(dict_get(info, "frames"));
    
    return true;
}

static Dictionary_t *_texture_getSubTexInfoDict(Texture_t *aTexture, const char *aTexName)
{
    dynamo_assert(aTexture->subtextures != NULL, "The texture contains no packing info");
    Dictionary_t *info = dict_get(aTexture->subtextures, aTexName);
    dynamo_assert(info != NULL, "Subtexture %s not found", aTexName);
    if(!info) return NULL;
    
    Dictionary_t *frameDef = dict_get(info, "frame");
    dynamo_assert(frameDef != NULL, "Invalid texture packing data!");
    
    return frameDef;
}

TextureRect_t texture_getSubTextureRect(Texture_t *aTexture, const char *aTexName)
{
    Dictionary_t *frameDef = _texture_getSubTexInfoDict(aTexture, aTexName);
    if(!frameDef)
        return textureRectangle_create(-1, -1, 0, 0);
    
    vec2_t origin = vec2_create(((Number_t*)dict_get(frameDef, "x"))->floatValue,
                                ((Number_t*)dict_get(frameDef, "y"))->floatValue);
    vec2_t size   = vec2_create(((Number_t*)dict_get(frameDef, "w"))->floatValue,
                                ((Number_t*)dict_get(frameDef, "h"))->floatValue);
    
    return textureRectangle_createWithPixelCoordinates(aTexture, origin, size);
}

vec2_t texture_getSubTextureOrigin(Texture_t *aTexture, const char *aTexName)
{
    Dictionary_t *frameDef = _texture_getSubTexInfoDict(aTexture, aTexName);
    if(!frameDef)
        return vec2_create(-1, -1);
    
    float texHeight = aTexture->size.h;
    float subTexHeight = ((Number_t*)dict_get(frameDef, "h"))->floatValue;
    vec2_t origin = vec2_create(((Number_t*)dict_get(frameDef, "x"))->floatValue,
                                ((Number_t*)dict_get(frameDef, "y"))->floatValue);
    origin.y = texHeight - origin.y - subTexHeight;
    
    return origin;
}

vec2_t texture_getSubTextureSize(Texture_t *aTexture, const char *aTexName)
{
    Dictionary_t *frameDef = _texture_getSubTexInfoDict(aTexture, aTexName);
    if(!frameDef)
        return vec2_create(-1, -1);
    
    return vec2_create(((Number_t*)dict_get(frameDef, "w"))->floatValue,
                       ((Number_t*)dict_get(frameDef, "h"))->floatValue);
}

TextureAtlas_t *texture_getSubTextureAtlas(Texture_t *aTexture, const char *aTexName, vec2_t aAtlasSize)
{
    vec2_t origin = texture_getSubTextureOrigin(aTexture, aTexName);
    dynamo_assert(origin.x > -1.0 && origin.y > -1.0, "Atlas not found in texture");
    
    return texAtlas_create(aTexture, origin, aAtlasSize);
}