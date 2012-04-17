#include "menu.h"
#include "shared.h"
#include "engine/various.h"
#include "engine/texture.h"
#include "engine/drawutils.h"

static void mainMenu_destroy(MainMenu_t *aMenu);

static void _mainMenu_draw(Renderer_t *aRenderer, void *aOwner, double aTimeSinceLastFrame, double aInterpolation);
static void _upKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData);
static void _downKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData);
static void _enterKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData);


MainMenu_t *mainMenu_create()
{
	MainMenu_t *out = obj_create_autoreleased(sizeof(MainMenu_t), (Obj_destructor_t)&mainMenu_destroy);
	out->numberOfItems = 2;
	out->selectedItem = 1;
	out->selectionCallback = NULL;
	out->fadingOut = false;
	out->fadeCallback = NULL;
	out->opacity = 1.0;
	out->disabled = false;

	out->renderable.displayCallback = &_mainMenu_draw;
	out->renderable.owner = out;

	// Load the menu texture
	Texture_t *itemTexture = obj_retain(texture_loadFromPng("textures/mainmenu.png", false, false));
	out->itemAtlas = obj_retain(texAtlas_create(itemTexture, kVec2_zero, vec2_create(54.0f, 10.0f)));

	// Load the background
	out->background = obj_retain(background_create());
	out->backgroundVelocity = vec2_create(15.0f, -28.0f);
	char *path;
	path = "textures/backgrounds/stars1.png";
	background_setLayer(out->background, 0, background_createLayer(texture_loadFromPng(path, true, true), 0.4f));
	path = "textures/backgrounds/stars2.png";
	background_setLayer(out->background, 1, background_createLayer(texture_loadFromPng(path, true, true), 0.7f));
	path = "textures/backgrounds/stars3.png";
	background_setLayer(out->background, 2, background_createLayer(texture_loadFromPng(path, true, true), 0.5));
	path = "textures/backgrounds/stars4.png";
	background_setLayer(out->background, 3, background_createLayer(texture_loadFromPng(path, true, true), 0.4));

	// Subscribe to keypresses
	out->arrowUpObserver = input_createObserver(kInputKey_arrowUp, &_upKeyPressed, NULL, out);
	out->arrowDownObserver = input_createObserver(kInputKey_arrowDown, &_downKeyPressed, NULL, out);
	out->enterObserver = input_createObserver(kInputKey_ascii, &_enterKeyPressed, "\n", out);
	input_addObserver(gInputManager, out->arrowUpObserver);
	input_addObserver(gInputManager, out->arrowDownObserver);
	input_addObserver(gInputManager, out->enterObserver);

	return out;
}

void mainMenu_destroy(MainMenu_t *aMenu)
{
	input_removeObserver(gInputManager, aMenu->arrowUpObserver);
	aMenu->arrowUpObserver = NULL;
	input_removeObserver(gInputManager, aMenu->arrowDownObserver);
	aMenu->arrowDownObserver = NULL;
	input_removeObserver(gInputManager, aMenu->enterObserver);
	aMenu->enterObserver = NULL;
	obj_release(aMenu->itemAtlas);
	aMenu->itemAtlas = NULL;
	obj_release(aMenu->background);
	aMenu->background = NULL;
}

void mainMenu_fadeOut(MainMenu_t *aMenu)
{
	aMenu->opacity = 1.0f;
	aMenu->fadingOut = true;
}

void mainMenu_update(MainMenu_t *aMenu, double aTimeDelta)
{
	if(aMenu->fadingOut) {
		aMenu->opacity = MAX(aMenu->opacity - 0.05, 0.0f);
		if(aMenu->opacity == 0.0f) {
			if(aMenu->fadeCallback) aMenu->fadeCallback(aMenu, aMenu->metaData);
			aMenu->fadingOut = false;
		}
	}

	aMenu->background->offset = vec2_add(aMenu->background->offset, vec2_scalarMul(aMenu->backgroundVelocity, aTimeDelta));
}

#pragma mark - Input handling

static void _upKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData)
{
	MainMenu_t *menu = (MainMenu_t *)metaData;
	if(menu->disabled || aState != kInputState_up) return;
	menu->selectedItem += 1;
	if(menu->selectedItem >= menu->numberOfItems) menu->selectedItem = 0;
}
static void _downKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData)
{
	MainMenu_t *menu = (MainMenu_t *)metaData;
	if(menu->disabled || aState != kInputState_up) return;
	menu->selectedItem -= 1;
	if(menu->selectedItem < 0) menu->selectedItem = menu->numberOfItems - 1;
}
static void _enterKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData)
{
	MainMenu_t *menu = (MainMenu_t *)metaData;
	if(menu->disabled || aState != kInputState_up) return;
	if(menu->selectionCallback)
		menu->selectionCallback(menu, menu->selectedItem, menu->metaData);
}

#pragma mark - Drawing

static void _mainMenu_draw(Renderer_t *aRenderer, void *aOwner, double aTimeSinceLastFrame, double aInterpolation)
{
	glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	MainMenu_t *menu = (MainMenu_t *)aOwner;

	menu->background->renderable.displayCallback(aRenderer, menu->background, aTimeSinceLastFrame, aInterpolation);

	float itemSpacing = 15.0f;
	vec2_t viewport = aRenderer->viewportSize;
	vec3_t center = { viewport.w / 2.0f, viewport.h/2.0 - (float)menu->numberOfItems * ((10.0f + itemSpacing)/2.0f), 0.0f };
	for(int i = 0; i < menu->numberOfItems; ++i) {
		int idx = i*2; // Row in texture atlas
		if(i == menu->selectedItem) ++idx;

		TextureRect_t texRect = texAtlas_getTextureRect(menu->itemAtlas, 0, idx);
		draw_texturePortion(center, menu->itemAtlas->texture, texRect, 1.0f, 0.0f, false, false);

		center.y += 10.0f + itemSpacing;
	}

	// Draw a black layer on top if fading out
	matrix_stack_push_item(aRenderer->worldMatrixStack, kMat4_identity);
	matrix_stack_push_item(aRenderer->projectionMatrixStack, kMat4_identity);
		vec4_t color = { 0.0f, 0.0f, 0.0f, 1.0f - menu->opacity };
		draw_rect(rect_create(-1.0f, -1.0f, 1.0f, 1.0f), 0.0f, color, true);
	matrix_stack_pop(aRenderer->projectionMatrixStack);
	matrix_stack_pop(aRenderer->worldMatrixStack);
}
