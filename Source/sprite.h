// Sprite class supporting atlases with multiple animations consisting of multiple frames
// Y Offset: Animations, X Offset: Frames

#include "object.h"
#include "renderer.h"
#include "texture_atlas.h"

#ifndef _SPRITE_H_
#define _SPRITE_H_

/*!
	Description of a single animation in a sprite's atlas.
*/
typedef struct _SpriteAnimation {
	int numberOfFrames;
	int currentFrame;
	bool loops;
} SpriteAnimation_t;

/*!
	A sprite

	@field atlas The sprite's texture atlas
	@field location The location at which the sprite is drawn in space.
	@field size The size of the sprite.
	@field scale The scale at which the sprite is drawn.
	@field angle The angle at which the sprite is drawn.
	@field flippedVertically Indicates whether or not the sprite is drawn flipped over the X axis.
	@field flippedHorizontally Indicates whether or not the sprite is drawn flipped over the Y axis.
	@field activeAnimation Indicates the active animation (0 being the bottom animation in an atlas)
*/
typedef struct _Sprite {
	OBJ_GUTS
    RENDERABLE_GUTS
	TextureAtlas_t *atlas;
	vec3_t location;
	vec2_t size;
	float scale, angle;
	bool flippedHorizontally;
	bool flippedVertically;
	int activeAnimation; // The y offset of the active animation
	SpriteAnimation_t *animations;
} Sprite_t;
extern Class_t Class_Sprite;

/*!
	Creates a sprite
*/
extern Sprite_t *sprite_create(vec3_t aLocation, vec2_t aSize, TextureAtlas_t *aAtlas, int aAnimationCapacity);
/*!
	Returns an animation with the specified number of frames, starting at frame 0 & looping.
*/
extern SpriteAnimation_t sprite_createAnimation(int aNumberOfFrames);

/*!
	Increments a sprite's animation frame.
*/
extern void sprite_step(Sprite_t *aSprite);
#endif
