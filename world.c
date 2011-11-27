#include "world.h"
#include "engine/sprite.h"
#include "engine/tmx_map.h"

static Sprite_t *sprite;

static void downKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData)
{
	sprite->angle += M_PI/20;
}
static void upKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData)
{
	static float scaleIncrementor = 0.0f;
	scaleIncrementor += M_PI/20;
	sprite->scale = 1.0f + 3.0*sinf(scaleIncrementor);
}

static void rightKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData)
{
	World_t *world = (World_t *)metaData;
	sprite->flippedHorizontally = false;
	if(world->ticks%2 == 0) sprite_step(sprite);

	world->background->offset.x += 6.0;
	world->background->offset.y = floorf(8.0f+10.0f* sinf((float)world->ticks*M_PI/50.0f));
	sprite->location.y = floorf(41.0f-world->background->offset.y);
}
static void leftKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData)
{
	World_t *world = (World_t *)metaData;
	sprite->flippedHorizontally = true;
	if(world->ticks%2 == 0) sprite_step(sprite);

	world->background->offset.x -= 6.0;
	world->background->offset.y = floorf(8.0f+10.0f* cosf((float)world->ticks*M_PI/50.0f));
	sprite->location.y = floorf(41.0f-world->background->offset.y);
}

static void mouseMoved(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData)
{
	World_t *world = (World_t *)metaData;
	static vec2_t lastLoc;
	static bool shouldResetDelta = true;
	vec2_t delta = kVec2_zero;
	if(!shouldResetDelta) delta = vec2_sub(*aLocation, lastLoc);

	world->background->offset = vec2_add(world->background->offset, delta);

	shouldResetDelta = (aState == kInputState_up);
	lastLoc = *aLocation;
}

World_t *world_init()
{
	World_t *out = malloc(sizeof(World_t));
	out->time = 0.0;
	out->ticks = 0;

	out->background = background_create();
	out->background->layers[0] = background_createLayer(texture_loadFromPng("textures/backgrounds/clouds.png", true, false), 0.5);
	out->background->layers[1] = background_createLayer(texture_loadFromPng("textures/backgrounds/hills.png", true, false), 0.3);
	out->background->layers[2] = background_createLayer(texture_loadFromPng("textures/backgrounds/castle.png", true, false), 0.1);
	out->background->layers[3] = background_createLayer(texture_loadFromPng("textures/backgrounds/ground.png", true, false), 0.0);
	renderer_pushRenderable(gRenderer, &out->background->renderable);

	vec3_t spriteLoc = { 100.0f, 41.0f, 0.0f };
	vec2_t spriteSize = { 48.0f, 48.0f };
	TextureAtlas_t *atlas = texAtlas_create(texture_loadFromPng("textures/sonic.png", false, false), kVec2_zero, spriteSize);
	sprite = sprite_create(spriteLoc, spriteSize, atlas, 1);
	sprite->animations[0] = sprite_createAnimation(11);
	renderer_pushRenderable(gRenderer, &sprite->renderable);

	out->level = level_load("levels/desert.tmx");
	//renderer_pushRenderable(gRenderer, &out->level->renderable);

	// Create&add input observers
	out->arrowRightObserver = input_createObserver(kInputKey_arrowRight, &rightKeyPressed, NULL, out);
	out->arrowLeftObserver = input_createObserver(kInputKey_arrowLeft, &leftKeyPressed, NULL, out);
	out->arrowUpObserver = input_createObserver(kInputKey_arrowUp, &upKeyPressed, NULL, out);
	out->arrowDownObserver = input_createObserver(kInputKey_arrowDown, &downKeyPressed, NULL, out);
	out->leftDragObserver = input_createObserver(kInputMouse_leftDrag, &mouseMoved, NULL, out);
	out->leftClickObserver = input_createObserver(kInputMouse_leftClick, &mouseMoved, NULL, out);

	input_addObserver(gInputManager, out->arrowRightObserver);
	input_addObserver(gInputManager, out->arrowLeftObserver);
	input_addObserver(gInputManager, out->arrowUpObserver);
	input_addObserver(gInputManager, out->arrowDownObserver);
	input_addObserver(gInputManager, out->leftDragObserver);
	input_addObserver(gInputManager, out->leftClickObserver);

	return out;
}

void world_destroy(World_t *aWorld)
{
	input_removeObserver(gInputManager, aWorld->arrowRightObserver);
	input_removeObserver(gInputManager, aWorld->arrowLeftObserver);
	input_removeObserver(gInputManager, aWorld->arrowUpObserver);
	input_removeObserver(gInputManager, aWorld->arrowDownObserver);
	input_removeObserver(gInputManager, aWorld->leftClickObserver);
	input_removeObserver(gInputManager, aWorld->leftDragObserver);

	background_destroy(aWorld->background, true);
	free(aWorld);
}

void world_update(World_t *aWorld, double aTimeDelta)
{
	aWorld->time += aTimeDelta;
	++aWorld->ticks;
}
