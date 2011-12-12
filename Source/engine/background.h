// Draws a background
// Supports up to 4 parallax layers

// Note: Assumes all layers have textures of the same size

#include "renderer.h"
#include "texture.h"

#ifndef _BACKGROUND_H_
#define _BACKGROUND_H_

#define kBackground_maxLayers (4)

typedef struct _BackgroundLayer {
	Texture_t *texture;
	float depth;
} BackgroundLayer_t;

typedef struct _Background {
	Renderable_t renderable;
	BackgroundLayer_t *layers[kBackground_maxLayers];
	vec2_t offset;
} Background_t;

extern Background_t *background_create();
extern void background_destroy(Background_t *aBackground, bool aShouldDestroyLayersAndTextures);

extern BackgroundLayer_t *background_createLayer(Texture_t *aTexture, float aDepth);
extern void background_destroyLayer(BackgroundLayer_t *aLayer, bool aShouldDestroyTexture);
#endif
