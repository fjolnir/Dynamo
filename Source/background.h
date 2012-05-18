/*!
	@header Background
	@abstract
	@discussion Draws a background
	Supports up to 4 parallax layers

	<b>Note:</b> Assumes all layers have textures of the same size
*/

#ifndef _BACKGROUND_H_
#define _BACKGROUND_H_

#include "renderer.h"
#include "texture.h"
#include "object.h"

extern Class_t Class_Background;
extern Class_t Class_BackgroundLayer;

#define kBackground_maxLayers (4)

/*!
	A background layer.

	@field texture The image drawn in the layer.
	@field depth The Z depth of the layer. Affects the parallax scrolling speed.
*/
typedef struct _BackgroundLayer {
	OBJ_GUTS;
	Texture_t *texture;
	float depth;
} BackgroundLayer_t;

/*!
	A background object.
	Can hold up to 4 layers.

	@field layers The layers to be drawn in the background
	@field offset The offset at which the background is drawn. (For scrolling)
*/
typedef struct _Background {
	OBJ_GUTS;
	RENDERABLE_GUTS;
	BackgroundLayer_t *layers[kBackground_maxLayers];
	vec2_t offset;
} Background_t;

/*!
	Creates a background object
*/
extern Background_t *background_create();
/*!
	Assigns a layer to the specified slot in the background.
	@param aIndex The index of the slot to assign to.
	@param aLayer The layer to assign.
*/
extern void background_setLayer(Background_t *aBackground, unsigned int aIndex, BackgroundLayer_t *aLayer);

/*!
	Creates a background layer object
	@param aTexture The texture to draw in the layer.
	@param aDepth The Z depth of the layer. Affects the parallax scrolling speed.
*/
extern BackgroundLayer_t *background_createLayer(Texture_t *aTexture, float aDepth);
#endif
