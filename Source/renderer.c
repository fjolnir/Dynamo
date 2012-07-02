#include "renderer.h"
#include "glutils.h"
#include "util.h"
#include "luacontext.h"

static void renderer_destroy(Renderer_t *aRenderer);
Class_t Class_Renderer = {
    "Renderer",
    sizeof(Renderer_t),
    (Obj_destructor_t)&renderer_destroy
};

Class_t Class_Renderable = {
    "Renderable",
    sizeof(Renderable_t),
    NULL
};

#pragma mark - Creation

Renderer_t *renderer_create(vec2_t aViewPortSize, vec3_t aCameraOffset)
{
    Renderer_t *out = obj_create_autoreleased(&Class_Renderer);
    out->renderables = obj_retain(llist_create((InsertionCallback_t)&obj_retain, (RemovalCallback_t)&obj_release));
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
    obj_release(aRenderer->renderables), aRenderer->renderables = NULL;
}


#pragma mark - Display

void renderer_display(Renderer_t *aRenderer, GLMFloat aTimeSinceLastFrame, GLMFloat aInterpolation)
{
    glClear(GL_COLOR_BUFFER_BIT);
    
    matrix_stack_push(aRenderer->worldMatrixStack);
    vec3_t ofs = aRenderer->cameraOffset;
    matrix_stack_translate(aRenderer->worldMatrixStack, ofs.x, ofs.y, ofs.z);
    
    // Render each of the renderer's entities
    LinkedListItem_t *currentItem = aRenderer->renderables->head;
    Renderable_t *renderable;
    if(currentItem) {
        do {
            renderable = (Renderable_t *)currentItem->value;
            if(renderable) {
                if(renderable->displayCallback)
                    renderable->displayCallback(aRenderer, renderable, aTimeSinceLastFrame, aInterpolation);
                if(renderable->luaDisplayCallback != -1) {
                    luaCtx_pushScriptHandler(GlobalLuaContext, renderable->luaDisplayCallback);
                    luaCtx_pushnumber(GlobalLuaContext, aTimeSinceLastFrame);
                    luaCtx_pushnumber(GlobalLuaContext, aInterpolation);
                    luaCtx_pcall(GlobalLuaContext, 2, 0, 0);
                }
            }
        } while((currentItem = currentItem->next));
    }
    
    matrix_stack_pop(aRenderer->worldMatrixStack);
}


#pragma mark - Entity list

void renderer_pushRenderable(Renderer_t *aRenderer, void *aRenderable)
{
    llist_pushValue(aRenderer->renderables, aRenderable);
}

void renderer_popRenderable(Renderer_t *aRenderer)
{
    llist_popValue(aRenderer->renderables);
}

bool renderer_insertRenderable(Renderer_t *aRenderer, void *aRenderableToInsert, void *aRenderableToShift)
{
    return llist_insertValue(aRenderer->renderables, aRenderableToInsert, aRenderableToShift);
}

bool renderer_deleteRenderable(Renderer_t *aRenderer, void *aRenderable)
{
    return llist_deleteValue(aRenderer->renderables, aRenderable);
}
