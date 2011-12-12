#include "world.h"
#include "engine/sprite.h"
#include "engine/tmx_map.h"


static void downKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData)
{
	World_t *world = (World_t *)metaData;
}
static void upKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData)
{
	World_t *world = (World_t *)metaData;
	world->level->character->collisionObject->velocity.y = -0.5f * world->level->collisionWorld->gravity.y;
}

static void rightKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData)
{
	World_t *world = (World_t *)metaData;
	world->level->character->sprite->flippedHorizontally = false;
	world->level->character->sprite->activeAnimation = aState == kInputState_down ? 5 : 6;
	world->level->character->collisionObject->velocity.x = 280.0f;
}
static void leftKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData)
{
	World_t *world = (World_t *)metaData;
	world->level->character->sprite->flippedHorizontally = true;
	world->level->character->sprite->activeAnimation = aState == kInputState_down ? 5 : 6;
	world->level->character->collisionObject->velocity.x = -280.0f;
}

static void characterCollided(CollisionWorld_t *aWorld, Collision_t collisionInfo)
{
	//debug_log("Character collided");
}

static void worldCollisionHappened(CollisionWorld_t *aWorld, Collision_t collisionInfo)
{
	//debug_log("SOMETHING COLLIDED!!");
}

static void mouseMoved(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData)
{
	static bool inDrag = false;
	World_t *world = (World_t *)metaData;
	static vec2_t lastLoc;
	static bool shouldResetDelta = true;
	vec2_t delta = kVec2_zero;
	if(!shouldResetDelta) delta = vec2_sub(*aLocation, lastLoc);


	if(aState == kInputState_up) {
		inDrag = false;
	}
	else if(aState == kInputState_down) {
		inDrag = true;
	}

	shouldResetDelta = (aState == kInputState_up);
	lastLoc = *aLocation;
}

World_t *world_init()
{
	World_t *out = malloc(sizeof(World_t));
	out->time = 0.0;
	out->ticks = 0;

	out->level = level_load("levels/spacetest.tmx");
	renderer_pushRenderable(gRenderer, &out->level->renderable);

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

	free(aWorld);
}

void world_update(World_t *aWorld, double aTimeDelta)
{
	aWorld->time += aTimeDelta;
	++aWorld->ticks;

	if(aWorld->level->character) {
		int timeSteps = 8;
		for(int i = 0; i < timeSteps; ++i)
			collision_step(aWorld->level->collisionWorld, aWorld->level->character->collisionObject, aTimeDelta/(float)timeSteps);
	}
}
