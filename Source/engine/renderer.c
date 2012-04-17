#include "renderer.h"
#include "glutils.h"
#include "various.h"

static void renderer_destroy(Renderer_t *aRenderer);

#pragma mark - Creation

Renderer_t *renderer_create(vec2_t aViewPortSize, vec3_t aCameraOffset)
{
	Renderer_t *out = obj_create_autoreleased(sizeof(Renderer_t), (Obj_destructor_t)&renderer_destroy);
	out->renderables = obj_retain(llist_create());
	out->frameBufferId = 0;
	out->viewportSize = aViewPortSize;
	out->cameraOffset = aCameraOffset;
	out->worldMatrixStack = matrix_stack_create(10);
	out->projectionMatrixStack = matrix_stack_create(10);

	// Initialize the transform matrices
	matrix_stack_push(out->worldMatrixStack);
	matrix_stack_push(out->projectionMatrixStack);

	return out;
}

void renderer_destroy(Renderer_t *aRenderer)
{
	obj_release(aRenderer->renderables);
	aRenderer->renderables = NULL;
}


#pragma mark - Display

void renderer_display(Renderer_t *aRenderer, double aTimeSinceLastFrame, double aInterpolation)
{
	glBindFramebuffer(GL_FRAMEBUFFER, aRenderer->frameBufferId);
	glViewport(0, 0, aRenderer->viewportSize.w, aRenderer->viewportSize.h);
	
	//glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT);
	
	// Render each of the renderer's entities
	LinkedListItem_t *currentItem = aRenderer->renderables->head;
	Renderable_t *renderable;
	if(currentItem) {
		do {
			renderable = (Renderable_t *)currentItem->value;
			renderable->displayCallback(aRenderer, renderable->owner, aTimeSinceLastFrame, aInterpolation);
		} while( (currentItem = currentItem->next) );
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


#pragma mark - Entity list

void renderer_pushRenderable(Renderer_t *aRenderer, Renderable_t *aRenderable)
{
	if(aRenderable->owner) obj_retain(aRenderable->owner);
	llist_pushValue(aRenderer->renderables, aRenderable);
}

void renderer_popRenderable(Renderer_t *aRenderer)
{
	Renderable_t *renderable = llist_popValue(aRenderer->renderables);
	if(renderable->owner) obj_release(renderable->owner);
}

bool renderer_insertRenderable(Renderer_t *aRenderer, Renderable_t *aRenderableToInsert, Renderable_t *aRenderableToShift)
{
	return llist_insertValue(aRenderer->renderables, aRenderableToInsert, aRenderableToShift);
}

bool renderer_deleteRenderable(Renderer_t *aRenderer, Renderable_t *aRenderable)
{
	bool didDelete = llist_deleteValue(aRenderer->renderables, aRenderable);
	if(didDelete && aRenderable->owner)
		obj_release(aRenderable->owner);
	return didDelete;
}
