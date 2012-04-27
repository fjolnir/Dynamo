#include "sprite.h"
#include "drawutils.h"
#include <stdlib.h>
#include "util.h"

static void sprite_destroy(Sprite_t *aSprite);
static void _sprite_draw(Renderer_t *aRenderer, Sprite_t *aSprite, GLMFloat aTimeSinceLastFrame, GLMFloat aInterpolation);

static Class_t Class_Sprite = {
	"Sprite",
	sizeof(Sprite_t),
	(Obj_destructor_t)&sprite_destroy
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
