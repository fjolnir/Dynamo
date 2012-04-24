// Draws a background
// Supports up to 4 parallax layers

// Note: Assumes all layers have textures of the same size

#include "renderer.h"
#include "texture.h"
#include "object.h"

#ifndef _BACKGROUND_H_
#define _BACKGROUND_H_

#define kBackground_maxLayers (4)

typedef struct _BackgroundLayer {
	OBJ_GUTS
	Texture_t *texture;
	float depth;
} BackgroundLayer_t;

typedef struct _Background {
	OBJ_GUTS
    RenderableDisplayCallback_t displayCallback;
	BackgroundLayer_t *layers[kBackground_maxLayers];
	vec2_t offset;
} Background_t;

extern Background_t *background_create();

extern void background_setLayer(Background_t *aBackground, unsigned int aIndex, BackgroundLayer_t *aLayer);
extern BackgroundLayer_t *background_createLayer(Texture_t *aTexture, float aDepth);
#endif
