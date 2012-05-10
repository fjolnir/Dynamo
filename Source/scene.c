#include "scene.h"

static void scene_destroy(Scene_t *self);
static void scene_draw(Renderer_t *aRenderer, Scene_t *aScene, GLMFloat aTimeSinceLastFrame, GLMFloat aInterpolation);

Class_t Class_Scene = {
	"Scene",
	sizeof(Scene_t),
	(Obj_destructor_t)&scene_destroy
};

Scene_t *scene_create(vec2_t aViewPortSize, vec3_t aCameraOffset)
{
	Scene_t *out = obj_create_autoreleased(&Class_Scene);
	out->transform = GLMMat4_identity;
    out->renderables = obj_retain(llist_create((InsertionCallback_t)&obj_retain, (RemovalCallback_t)&obj_release));
    out->displayCallback = (RenderableDisplayCallback_t)&scene_draw;
	return out;
}

static void scene_destroy(Scene_t *self)
{
    obj_release(self->renderables), self->renderables = NULL;
}

static void scene_draw(Renderer_t *aRenderer, Scene_t *aScene, GLMFloat aTimeSinceLastFrame, GLMFloat aInterpolation)
{
    matrix_stack_push(aRenderer->worldMatrixStack);
    matrix_stack_mul_mat4(aRenderer->worldMatrixStack, aScene->transform);
    LinkedListItem_t *item = aScene->renderables->head;
	if(item) {
		do {
            Renderable_t *renderable = item->value;
            renderable->displayCallback(aRenderer, renderable, aTimeSinceLastFrame, aInterpolation);
		} while( (item = item->next));
	}
    matrix_stack_pop(aRenderer->worldMatrixStack);
}

void scene_pushRenderable(Scene_t *aScene, void *aRenderable)
{
	llist_pushValue(aScene->renderables, aRenderable);
}

void scene_popRenderable(Scene_t *aScene)
{
	Renderable_t *renderable = llist_popValue(aScene->renderables);
}

bool scene_insertRenderable(Scene_t *aScene, void *aRenderableToInsert, void *aRenderableToShift)
{
	return llist_insertValue(aScene->renderables, aRenderableToInsert, aRenderableToShift);
}

bool scene_deleteRenderable(Scene_t *aScene, void *aRenderable)
{
	bool didDelete = llist_deleteValue(aScene->renderables, aRenderable);
	if(didDelete)
		obj_release(aRenderable);
	return didDelete;
}
