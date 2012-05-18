// Just a list of renderables encapsulated in a single drawable with a common transformation

#ifndef _SCENE_H_
#define _SCENE_H_

#include "object.h"
#include "glutils.h"
#include "GLMath/GLMath.h"
#include "linkedlist.h"
#include "renderer.h"

/*!
	A collection of renderables wit a common transformation

	@field transform A transformation applied to every renderable in the scene
*/
typedef struct _Scene {
    OBJ_GUTS
    RENDERABLE_GUTS
    LinkedList_t *renderables;
    mat4_t transform;
} Scene_t;
extern Class_t Class_Scene;

/*!
	Creates a scene
*/
extern Scene_t *scene_create();

/*!
	Adds a renderable to the top of the scene stack.
*/
extern void scene_pushRenderable(Scene_t *aScene, void *aRenderable);
/*!
	Removes the topmost renderable
*/
extern void scene_popRenderable(Scene_t *aScene);
/*!
	Inserts a renderable behind another renderable
*/
extern bool scene_insertRenderable(Scene_t *aScene, void *aRenderableToInsert, void *aRenderableToShift);
/*!
	Removes the given renderable from the renderable stack
*/
extern bool scene_deleteRenderable(Scene_t *aScene, void *aRenderable);
#endif
