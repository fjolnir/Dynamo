#include "sprite.h"
#include "drawutils.h"
#include <stdlib.h>
#include "util.h"

static void sprite_destroy(Sprite_t *aSprite);
static void _sprite_draw(Renderer_t *aRenderer, Sprite_t *aSprite, GLMFloat aTimeSinceLastFrame, GLMFloat aInterpolation);
static void spriteBatch_destroy(SpriteBatch_t *aBatch);
static void _spriteBatch_draw(Renderer_t *aRenderer, SpriteBatch_t *aBatch, GLMFloat aTimeSinceLastFrame, GLMFloat aInterpolation);

Class_t Class_Sprite = {
    "Sprite",
    sizeof(Sprite_t),
    (Obj_destructor_t)&sprite_destroy
};

Class_t Class_SpriteBatch = {
    "SpriteBatch",
    sizeof(SpriteBatch_t),
    (Obj_destructor_t)&spriteBatch_destroy
};

Sprite_t *sprite_create(vec3_t aLocation, vec2_t aSize, TextureAtlas_t *aAtlas, int aAnimationCapacity)
{
    Sprite_t *out = (Sprite_t*)obj_create_autoreleased(&Class_Sprite);
    out->displayCallback = (RenderableDisplayCallback_t)&_sprite_draw;
    out->luaDisplayCallback = -1;
    out->location = aLocation;
    out->size = aSize;
    out->scale = 1.0f;
    out->angle = 0.0f;
    out->opacity = 1.0f;
    out->atlas = obj_retain(aAtlas);
    out->flippedHorizontally = false;
    out->flippedVertically = false;
    out->activeAnimation = 0;
    out->animations = calloc(aAnimationCapacity, sizeof(SpriteAnimation_t));

    return out;
}
void sprite_destroy(Sprite_t *aSprite)
{
    obj_release(aSprite->atlas);
    aSprite->atlas = NULL;
    free(aSprite->animations);
    aSprite->animations = NULL;
}

SpriteAnimation_t sprite_createAnimation(int aNumberOfFrames)
{
    SpriteAnimation_t out = { aNumberOfFrames, 0, true };
    return out;
}


#pragma mark - Animation

void sprite_step(Sprite_t *aSprite)
{
    SpriteAnimation_t *animation = &aSprite->animations[aSprite->activeAnimation];
    animation->currentFrame += 1;
    if(animation->currentFrame >= animation->numberOfFrames) {
        if(animation->loops)
            animation->currentFrame = 0;
        else
            animation->currentFrame -= 1;
    }
}

#pragma mark - Rendering

void _sprite_draw(Renderer_t *aRenderer, Sprite_t *aSprite, GLMFloat aTimeSinceLastFrame, GLMFloat aInterpolation)
{
    SpriteAnimation_t *animation = &aSprite->animations[aSprite->activeAnimation];

    TextureRect_t cropRect = texAtlas_getTextureRect(aSprite->atlas, animation->currentFrame, aSprite->activeAnimation);
    draw_texturePortion(aSprite->location, aSprite->atlas->texture, cropRect, aSprite->scale, aSprite->angle, aSprite->opacity, aSprite->flippedHorizontally, aSprite->flippedVertically);
}


#pragma mark - - Batches

struct _BatchVertex {
    vec3_t loc;
    vec2_t texCoord;
};

SpriteBatch_t *spriteBatch_create(TextureAtlas_t *aAtlas)
{
    SpriteBatch_t *out = obj_create_autoreleased(&Class_SpriteBatch);
    out->spriteCount = 0;
    out->sprites = obj_retain(llist_create((InsertionCallback_t)&obj_retain, (RemovalCallback_t)&obj_release));
    out->displayCallback = (RenderableDisplayCallback_t)&_spriteBatch_draw;
    out->luaDisplayCallback = -1;
    
    glGenBuffers(2, &out->vbo);
    
    return out;
}

void spriteBatch_destroy(SpriteBatch_t *aBatch)
{
    glDeleteBuffers(2, &aBatch->vbo);
    obj_release(aBatch->sprites);
}

static void _spriteBatch_updateVbo(SpriteBatch_t *aBatch);

void _spriteBatch_draw(Renderer_t *aRenderer, SpriteBatch_t *aBatch, GLMFloat aTimeSinceLastFrame, GLMFloat aInterpolation)
{
    if(aBatch->spriteCount == 0)
        return;
    
    LinkedListItem_t *item = aBatch->sprites->head;
    if(!item) return;
    Sprite_t *firstSprite = item->value;
    Texture_t *tex = firstSprite->atlas->texture;
    _spriteBatch_updateVbo(aBatch);

    shader_makeActive(gTexturedShader);
    shader_updateMatrices(gTexturedShader, aRenderer);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    glUniform1i(gTexturedShader->uniforms[kShader_colormap0Uniform], 0);
    GLfloat white[4] = { 1,1,1,1 };
    glUniform4fv(gTexturedShader->uniforms[kShader_colorUniform], 1, white);
    
    glBindBuffer(GL_ARRAY_BUFFER, aBatch->vbo);
    glVertexAttribPointer(gTexturedShader->attributes[kShader_positionAttribute], 3, GL_FLOAT, GL_FALSE, sizeof(struct _BatchVertex), (void*)offsetof(struct _BatchVertex, loc));
    glEnableVertexAttribArray(gTexturedShader->attributes[kShader_positionAttribute]);
    glVertexAttribPointer(gTexturedShader->attributes[kShader_texCoord0Attribute], 2, GL_FLOAT, GL_FALSE, sizeof(struct _BatchVertex), (void*)offsetof(struct _BatchVertex, texCoord));
    glEnableVertexAttribArray(gTexturedShader->attributes[kShader_texCoord0Attribute]);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, aBatch->ibo);
    glDrawElements(GL_TRIANGLE_STRIP, aBatch->idxCount, GL_UNSIGNED_SHORT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    shader_makeInactive(gTexturedShader);
}

void _spriteBatch_updateVbo(SpriteBatch_t *aBatch)
{
    if(aBatch->spriteCount == 0)
        aBatch->idxCount = 0;
    
    LinkedListItem_t *item = aBatch->sprites->head;
    if(!item) return;
    
    Sprite_t *sprite = item->value;
    SpriteAnimation_t *animation;
    TextureRect_t cropRect;
    TextureAtlas_t *atlas;
    float maxTexX, maxTexY;
    quat_t rot;

    unsigned idxCount = aBatch->spriteCount * 4;
    unsigned indexCount = aBatch->spriteCount * 5 + (aBatch->spriteCount - 2);
    glBindBuffer(GL_ARRAY_BUFFER, aBatch->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, aBatch->ibo);
    if(idxCount > aBatch->idxCapacity || aBatch->idxCapacity - idxCount > (aBatch->idxCapacity/2)) {
        glBufferData(GL_ARRAY_BUFFER, idxCount*sizeof(struct _BatchVertex), NULL, GL_DYNAMIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount*sizeof(GLushort), NULL, GL_DYNAMIC_DRAW);
        aBatch->idxCapacity = idxCount;
    }
    
    struct _BatchVertex *vertices = glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
    GLushort *indices = glMapBufferOES(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY_OES);
    
    int i = 0, iIdx = 0;
    do {
        sprite = item->value;
        atlas = sprite->atlas;
        animation = &sprite->animations[sprite->activeAnimation];
        cropRect = texAtlas_getTextureRect(atlas, animation->currentFrame, sprite->activeAnimation);
        
        if(i > 0)
            indices[iIdx++] = i;
        indices[iIdx++] = i + 0;
        indices[iIdx++] = i + 1;
        indices[iIdx++] = i + 2;
        indices[iIdx++] = i + 3;
        if(i+3 < idxCount-1)
            indices[iIdx++] = i+3;
        
        // Transform the sprite into world position (We can't do this in the vertex shader since each sprite 
        // may have a different transformation)
        vertices[i+0].loc = (vec3_t){ -sprite->size.w/2, -sprite->size.h/2, 0 };
        vertices[i+1].loc = (vec3_t){ -sprite->size.w/2,  sprite->size.h/2, 0 };
        vertices[i+2].loc = (vec3_t){  sprite->size.w/2, -sprite->size.h/2, 0 };
        vertices[i+3].loc = (vec3_t){  sprite->size.w/2,  sprite->size.h/2, 0 };
        
        // Rotate if necessary
        if(fabs(sprite->angle) > 0.01) {
            rot = quat_createv((vec3_t){0,0,1}, sprite->angle);
            vertices[i+0].loc = quat_rotateVec3(rot, vertices[i+0].loc);
            vertices[i+1].loc = quat_rotateVec3(rot, vertices[i+1].loc);
            vertices[i+2].loc = quat_rotateVec3(rot, vertices[i+2].loc);
            vertices[i+3].loc = quat_rotateVec3(rot, vertices[i+3].loc);
        }
        // Scale if necessary
        if(sprite->scale != 1.0) {
            vertices[i+0].loc = vec3_scalarMul(vertices[i+0].loc, sprite->scale);
            vertices[i+1].loc = vec3_scalarMul(vertices[i+1].loc, sprite->scale);
            vertices[i+2].loc = vec3_scalarMul(vertices[i+2].loc, sprite->scale);
            vertices[i+3].loc = vec3_scalarMul(vertices[i+3].loc, sprite->scale);
        }
        
        vertices[i+0].loc = vec3_add(vertices[i+0].loc, sprite->location);
        vertices[i+1].loc = vec3_add(vertices[i+1].loc, sprite->location);
        vertices[i+2].loc = vec3_add(vertices[i+2].loc, sprite->location);
        vertices[i+3].loc = vec3_add(vertices[i+3].loc, sprite->location);
        
        
        maxTexX = cropRect.origin.x + cropRect.size.w;
        maxTexY = cropRect.origin.y + cropRect.size.h;
        
        vertices[i+0].texCoord = (vec2_t) {
            sprite->flippedHorizontally ? maxTexX           : cropRect.origin.x,
            sprite->flippedVertically   ? maxTexY           : cropRect.origin.y };
        vertices[i+1].texCoord = (vec2_t) {
            sprite->flippedHorizontally ? maxTexX           : cropRect.origin.x,
            sprite->flippedVertically   ? cropRect.origin.y : maxTexY };
        vertices[i+2].texCoord = (vec2_t) {
            sprite->flippedHorizontally ? cropRect.origin.x : maxTexX,
            sprite->flippedVertically   ? maxTexY           : cropRect.origin.y };
        vertices[i+3].texCoord = (vec2_t) {
            sprite->flippedHorizontally ? cropRect.origin.x : maxTexX,
            sprite->flippedVertically   ? cropRect.origin.y : maxTexY };
        
        i += 4;
    } while((item = item->next));

    glUnmapBufferOES(GL_ARRAY_BUFFER);
    glUnmapBufferOES(GL_ELEMENT_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    aBatch->idxCount = indexCount;
}

void spriteBatch_addSprite(SpriteBatch_t *aBatch, Sprite_t *aSprite)
{
    llist_pushValue(aBatch->sprites, aSprite);
    ++aBatch->spriteCount;
}

bool spriteBatch_insertSprite(SpriteBatch_t *aBatch, Sprite_t *aSprite, Sprite_t *aSpriteToShift)
{
    bool ret = llist_insertValue(aBatch->sprites, aSprite, aSpriteToShift);
    ++aBatch->spriteCount;
    return ret;
}

bool spriteBatch_deleteSprite(SpriteBatch_t *aBatch, Sprite_t *aSprite)
{
    bool ret = llist_deleteValue(aBatch->sprites, aSprite);
    if(ret)
        --aBatch->spriteCount;
    return ret;
}