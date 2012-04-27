// Just a list of renderables encapsulated in a single drawable with a common transformation

#ifndef _SCENE_H_
#define _SCENE_H_

#include "object.h"
#include "glutils.h"
#include "GLMath/GLMath.h"
#include "linkedlist.h"
#include "renderer.h"

extern Class_t Class_Scene;
typedef struct _Scene {
    OBJ_GUTS
    RenderableDisplayCallback_t displayCallback;
    LinkedList_t *renderables;
    mat4_t transform;
} Scene_t;

extern Scene_t *scene_create();

// Scene object list management (Objects need to conform to the layout of Renderable_t)
extern void scene_pushRenderable(Scene_t *aScene, void *aRenderable);
extern void scene_popRenderable(Scene_t *aScene);
extern bool scene_insertRenderable(Scene_t *aScene, void *aRenderableToInsert, void *aRenderableToShift);
extern bool scene_deleteRenderable(Scene_t *aScene, void *aRenderable);

#endif
