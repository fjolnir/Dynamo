// Simple 2.5D renderer with a "camera"
// Draws the area defined by viewport.
// CameraOffset defines the center of the viewport

// The actual drawing of objects is handled by Renderables, whose display callbacks
// are implemented in their appropriate files.

#include "glutils.h"
#include "GLMath/GLMath.h"
#include "linkedlist.h"

#ifndef _RENDERER_H_
#define _RENDERER_H_

typedef struct _Renderer Renderer_t;

// For defining an object you wish to have rendered
typedef struct _Renderable {
	void (*displayCallback)(Renderer_t *aRenderer, void *aOwner, double aTimeSinceLastFrame, double aInterpolation);
	void *owner;
} Renderable_t;

// The renderer object
struct _Renderer {
	GLuint frameBufferId; // The FBO the renderer should draw to
	vec2_t viewportSize;
	vec3_t cameraOffset;
	matrix_stack_t *worldMatrixStack;
	matrix_stack_t *projectionMatrixStack;
	LinkedList_t *renderables; // For internal use only
};

// Renderer creation
extern Renderer_t *renderer_create(vec2_t aViewPortSize, vec3_t aCameraOffset);
extern void renderer_destroy(Renderer_t *aRenderer);

// Display
extern void renderer_display(Renderer_t *aRenderer, double aTimeSinceLastFrame, double aInterpolation);

// Renderable list management
extern void renderer_pushRenderable(Renderer_t *aRenderer, Renderable_t *aRenderable);
extern void renderer_popRenderable(Renderer_t *aRenderer);
extern bool renderer_insertRenderable(Renderer_t *aRenderer, Renderable_t *aRenderableToInsert, Renderable_t *aRenderableToShift);
extern bool renderer_deleteRenderable(Renderer_t *aRenderer, Renderable_t *aRenderable);

#endif
