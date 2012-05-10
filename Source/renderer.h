// Simple 2.5D renderer with a "camera"
// Draws the area defined by viewport.
// CameraOffset defines the center of the viewport

// The actual drawing of objects is handled by Renderables, whose display callbacks
// are implemented in their appropriate files.

#include "object.h"
#include "glutils.h"
#include "GLMath/GLMath.h"
#include "linkedlist.h"

#ifndef _RENDERER_H_
#define _RENDERER_H_

typedef struct _Renderer Renderer_t;
typedef struct _Renderable Renderable_t;

typedef void (*RenderableDisplayCallback_t)(Renderer_t *aRenderer, Renderable_t *aRenderable, GLMFloat aTimeSinceLastFrame, GLMFloat aInterpolation);

// To make an Object renderable, include RENDERABLE_GUTS immediately after OBJ_GUTS
#define RENDERABLE_GUTS \
    RenderableDisplayCallback_t displayCallback;

// For defining an object you wish to have rendered
// (You usually wouldn't use this type directly, rather you'd simply add RENDERABLE_GUTS
//  as the first field after the guts of the object you wish to render)
extern Class_t Class_Renderable;
struct _Renderable {
    OBJ_GUTS
    RENDERABLE_GUTS
};

// The renderer object
extern Class_t Class_Renderer;
struct _Renderer {
	OBJ_GUTS
	GLuint frameBufferId; // The FBO the renderer should draw to
	vec2_t viewportSize;
	vec3_t cameraOffset;
	matrix_stack_t *worldMatrixStack;
	matrix_stack_t *projectionMatrixStack;
	LinkedList_t *renderables; // For internal use only
};

// Renderer creation
extern Renderer_t *renderer_create(vec2_t aViewPortSize, vec3_t aCameraOffset);

// Display
extern void renderer_display(Renderer_t *aRenderer, GLMFloat aTimeSinceLastFrame, GLMFloat aInterpolation);

// Renderable list management (Renderables need to conform to the layout of Renderable_t). Retains
extern void renderer_pushRenderable(Renderer_t *aRenderer, void *aRenderable);
extern void renderer_popRenderable(Renderer_t *aRenderer);
extern bool renderer_insertRenderable(Renderer_t *aRenderer, void *aRenderableToInsert, void *aRenderableToShift);
extern bool renderer_deleteRenderable(Renderer_t *aRenderer, void *aRenderable);

#endif
