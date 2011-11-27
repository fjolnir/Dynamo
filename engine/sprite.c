#include "sprite.h"
#include "drawutils.h"
#include <stdlib.h>
#include "various.h"

static void _sprite_draw(Renderer_t *aRenderer, void *aOwner, double aTimeSinceLastFrame, double aInterpolation);

Sprite_t *sprite_create(vec3_t aLocation, vec2_t aSize, TextureAtlas_t *aAtlas, int aAnimationCapacity)
{
	Sprite_t *out = malloc(sizeof(Sprite_t));
	out->renderable.displayCallback = &_sprite_draw;	
	out->location = aLocation;
	out->size = aSize;
	out->scale = 1.0f;
	out->angle = 0.0f;
	out->atlas = aAtlas;
	out->flippedHorizontally = false;
	out->flippedVertically = false;
	out->activeAnimation = 0;
	out->animations = calloc(aAnimationCapacity, sizeof(SpriteAnimation_t));

	return out;
}
void sprite_destroy(Sprite_t *aSprite, bool aShouldDestroyAtlas, bool shouldDestroyTexture)
{
	if(aShouldDestroyAtlas) texAtlas_destroy(aSprite->atlas, shouldDestroyTexture);
	free(aSprite->animations);
	free(aSprite);
}

SpriteAnimation_t sprite_createAnimation(int aNumberOfFrames)
{
	SpriteAnimation_t out = { aNumberOfFrames, 0 };
	return out;
}


#pragma mark - Animation

void sprite_step(Sprite_t *aSprite)
{
	SpriteAnimation_t *animation = &aSprite->animations[aSprite->activeAnimation];
	animation->currentFrame += 1;
	if(animation->currentFrame >= animation->numberOfFrames) animation->currentFrame = 0;
}

#pragma mark - Rendering

static void _sprite_draw(Renderer_t *aRenderer, void *aOwner, double aTimeSinceLastFrame, double aInterpolation)
{
	Sprite_t *sprite = (Sprite_t *)aOwner;
	SpriteAnimation_t *animation = &sprite->animations[sprite->activeAnimation];

	TextureRect_t cropRect = texAtlas_getTextureRect(sprite->atlas, animation->currentFrame, sprite->activeAnimation);
	draw_texturePortion(sprite->location, sprite->atlas->texture, cropRect, sprite->scale, sprite->angle, sprite->flippedHorizontally, sprite->flippedVertically);
}
