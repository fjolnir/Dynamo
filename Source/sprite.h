/*!
    @header Sprite
    @abstract
    @discussion Sprite class supporting atlases with multiple animations consisting of multiple frames

    Y Offset in atlas: Animations, X Offset: Frames
*/

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
    float scale, angle, opacity;
    bool flippedHorizontally;
    bool flippedVertically;
    int activeAnimation; // The y offset of the active animation
    SpriteAnimation_t *animations;
} Sprite_t;
extern Class_t Class_Sprite;

// A sprite batch enables multiple sprites sharing a texture atlas to be drawn in a single draw call
// (Uses the atlas of the first sprite in the batch and does not support sprite opacity)
typedef struct _SpriteBatch {
    OBJ_GUTS
    RENDERABLE_GUTS
    int spriteCount;
    LinkedList_t *sprites;
    unsigned vbo, ibo, vertCount, vertCapacity;
} SpriteBatch_t;
extern Class_t Class_SpriteBatch;

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

/*!
 Creates a sprite batch
*/
extern SpriteBatch_t *spriteBatch_create();
/*!
 Adds the given sprite to the batch.
*/
extern void spriteBatch_addSprite(SpriteBatch_t *aBatch, Sprite_t *aSprite);

/*!
 Inserts aSprite before aSpriteToShift
 */
extern bool spriteBatch_insertSprite(SpriteBatch_t *aBatch, Sprite_t *aSprite, Sprite_t *aSpriteToShift);

/*!
 Removes the given sprite from the batch.
*/
extern bool spriteBatch_deleteSprite(SpriteBatch_t *aBatch, Sprite_t *aSprite);
#endif
