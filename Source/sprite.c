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
	out->location = aLocation;
	out->size = aSize;
	out->scale = 1.0f;
	out->angle = 0.0f;
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
	draw_texturePortion(aSprite->location, aSprite->atlas->texture, cropRect, aSprite->scale, aSprite->angle, aSprite->flippedHorizontally, aSprite->flippedVertically);
}


#pragma mark - - Batches

SpriteBatch_t *spriteBatch_create(TextureAtlas_t *aAtlas)
{
    SpriteBatch_t *out = obj_create_autoreleased(&Class_SpriteBatch);
    out->spriteCount = 0;
    out->sprites = obj_retain(llist_create((InsertionCallback_t)&obj_retain, (RemovalCallback_t)&obj_release));
    out->displayCallback = (RenderableDisplayCallback_t)&_spriteBatch_draw;
	return out;
}

void spriteBatch_destroy(SpriteBatch_t *aBatch)
{
    obj_release(aBatch->sprites);
}

struct _BatchVertex {
    vec3_t loc;
    vec2_t texCoord;
};
void _spriteBatch_draw(Renderer_t *aRenderer, SpriteBatch_t *aBatch, GLMFloat aTimeSinceLastFrame, GLMFloat aInterpolation)
{
    if(aBatch->spriteCount == 0)
        return;
    
    LinkedListItem_t *item = aBatch->sprites->head;
    if(!item) return;
    
    Sprite_t *sprite = item->value;
    SpriteAnimation_t *animation;
    TextureRect_t cropRect;
    Texture_t *tex = sprite->atlas->texture;
    TextureAtlas_t *atlas;
    float maxTexX, maxTexY;
    quat_t rot;
    
    unsigned vertCount = aBatch->spriteCount * 5 + (aBatch->spriteCount - 2);
    struct _BatchVertex *vertices = malloc(vertCount*sizeof(struct _BatchVertex));
    int i = 0;
    int ofs = 0;
    do {
        sprite = item->value;
        atlas = sprite->atlas;
        animation = &sprite->animations[sprite->activeAnimation];
        cropRect = texAtlas_getTextureRect(atlas, animation->currentFrame, sprite->activeAnimation);
        
        // Transform the sprite into world position (We can't do this in the vertex shader since each sprite may have
        // a different transformation)
        if(i > 0)
            ofs = i + 1;
        else
            ofs = 0;
        vertices[ofs+0].loc = (vec3_t){ -sprite->size.w/2, -sprite->size.h/2, 0 };
        vertices[ofs+1].loc = (vec3_t){ -sprite->size.w/2,  sprite->size.h/2, 0 };
        vertices[ofs+2].loc = (vec3_t){  sprite->size.w/2, -sprite->size.h/2, 0 };
        vertices[ofs+3].loc = (vec3_t){  sprite->size.w/2,  sprite->size.h/2, 0 };
        
        // Rotate if necessary
        if(fabs(sprite->angle) > 0.01) {
            rot = quat_createv((vec3_t){0,0,1}, sprite->angle);
            vertices[ofs+0].loc = quat_rotateVec3(rot, vertices[ofs+0].loc);
            vertices[ofs+1].loc = quat_rotateVec3(rot, vertices[ofs+1].loc);
            vertices[ofs+2].loc = quat_rotateVec3(rot, vertices[ofs+2].loc);
            vertices[ofs+3].loc = quat_rotateVec3(rot, vertices[ofs+3].loc);
        }
        
        vertices[ofs+0].loc = vec3_add(vertices[ofs+0].loc, sprite->location);
        vertices[ofs+1].loc = vec3_add(vertices[ofs+1].loc, sprite->location);
        vertices[ofs+2].loc = vec3_add(vertices[ofs+2].loc, sprite->location);
        vertices[ofs+3].loc = vec3_add(vertices[ofs+3].loc, sprite->location);
        
        // Create deformed triangles to split apart the sprites
        if(i > 0)
            vertices[i].loc = vertices[ofs].loc;
        if(ofs+3 < vertCount-1)
            vertices[ofs+4].loc = vertices[ofs+3].loc;

        maxTexX = cropRect.origin.x + cropRect.size.w;
        maxTexY = cropRect.origin.y + cropRect.size.h;

        vertices[ofs+0].texCoord = (vec2_t) { sprite->flippedHorizontally ? maxTexX : cropRect.origin.x, sprite->flippedVertically ? maxTexY : cropRect.origin.y };
        vertices[ofs+1].texCoord = (vec2_t) { sprite->flippedHorizontally ? maxTexX : cropRect.origin.x, sprite->flippedVertically ? cropRect.origin.y : maxTexY };
        vertices[ofs+2].texCoord = (vec2_t) { sprite->flippedHorizontally ? cropRect.origin.x : maxTexX, sprite->flippedVertically ? maxTexY : cropRect.origin.y };
        vertices[ofs+3].texCoord = (vec2_t) { sprite->flippedHorizontally ? cropRect.origin.x : maxTexX, sprite->flippedVertically ? cropRect.origin.y : maxTexY };

        i += (i==0 || i == vertCount-6) ? 5 : 6;
    } while((item = item->next));
    
    shader_makeActive(gTexturedShader);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex->id);
    
    shader_updateMatrices(gTexturedShader, aRenderer);
    glUniform1i(gTexturedShader->uniforms[kShader_colormap0Uniform], 0);
    GLfloat white[4] = { 1,1,1,1 };
    glUniform4fv(gTexturedShader->uniforms[kShader_colorUniform], 1, white);
    
    glVertexAttribPointer(gTexturedShader->attributes[kShader_positionAttribute], 3, GL_FLOAT, GL_FALSE, sizeof(struct _BatchVertex), &vertices->loc);
    glEnableVertexAttribArray(gTexturedShader->attributes[kShader_positionAttribute]);
    glVertexAttribPointer(gTexturedShader->attributes[kShader_texCoord0Attribute], 2, GL_FLOAT, GL_FALSE, sizeof(struct _BatchVertex), &vertices->texCoord);
    glEnableVertexAttribArray(gTexturedShader->attributes[kShader_texCoord0Attribute]);
    
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertCount);
    
    shader_makeInactive(gTexturedShader);    
    glBindTexture(GL_TEXTURE_2D, 0);
    free(vertices);
}

void spriteBatch_addSprite(SpriteBatch_t *aBatch, Sprite_t *aSprite)
{
    llist_pushValue(aBatch->sprites, aSprite);
    ++aBatch->spriteCount;
}

bool spriteBatch_insertSprite(SpriteBatch_t *aBatch, Sprite_t *aSprite, Sprite_t *aSpriteToShift)
{
    bool ret = llist_insertValue(aBatch->sprites, aSprite, aSpriteToShift);
    if(!ret)
        return false;
    ++aBatch->spriteCount;
    return true;
}

bool spriteBatch_deleteSprite(SpriteBatch_t *aBatch, Sprite_t *aSprite)
{
    bool ret = llist_deleteValue(aBatch->sprites, aSprite);
    if(ret)
        --aBatch->spriteCount;
    return ret;
}