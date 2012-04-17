// Sprite class supporting atlases with multiple animations consisting of multiple frames
// Y Offset: Animations, X Offset: Frames

#include "object.h"
#include "renderer.h"
#include "texture_atlas.h"

#ifndef _SPRITE_H_
#define _SPRITE_H_

typedef struct _SpriteAnimation {
	int numberOfFrames;
	int currentFrame;
	bool loops;
} SpriteAnimation_t;

typedef struct _Sprite {
	OBJ_GUTS
	Renderable_t renderable;
	TextureAtlas_t *atlas;
	vec3_t location;
	vec2_t size;
	float scale, angle;
	bool flippedHorizontally;
	bool flippedVertically;
	int activeAnimation; // The y offset of the active animation
	SpriteAnimation_t *animations;
} Sprite_t;

extern Sprite_t *sprite_create(vec3_t aLocation, vec2_t aSize, TextureAtlas_t *aAtlas, int aAnimationCapacity);
extern SpriteAnimation_t sprite_createAnimation(int aNumberOfFrames);

extern void sprite_step(Sprite_t *aSprite);
#endif
