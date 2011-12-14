#include "world.h"
#include "engine/sprite.h"
#include "engine/tmx_map.h"


static void downKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData)
{
	World_t *world = (World_t *)metaData;
	Character_t *character = world->level->character;
	world->level->collisionWorld->gravity.y *= -1.0f;
	if(character->collisionObject->inContact) {
		character->sprite->animations[5].currentFrame = 0;
		character->sprite->activeAnimation = 5;
	}
}
static void upKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData)
{
	World_t *world = (World_t *)metaData;
	Character_t *character = world->level->character;
	character->collisionObject->velocity.y = -0.5f * world->level->collisionWorld->gravity.y;
	if(character->collisionObject->inContact) {
		character->sprite->animations[5].currentFrame = 0;
		character->sprite->activeAnimation = 5;
	}
}

static void rightKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData)
{
	World_t *world = (World_t *)metaData;
	vec2_t grav = world->level->collisionWorld->gravity;
	world->level->character->sprite->flippedHorizontally = grav.y > 0.0f;
	CollisionPolyObject_t *collObj = world->level->character->collisionObject;
	if(world->level->character->collisionObject->inContact) {
		world->level->character->sprite->activeAnimation = aState == kInputState_down ? 6 : 7;
		collObj->velocity.x = 280.0f;
	} else
		collObj->velocity.x += 20.0f;
}

static void leftKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData)
{
	World_t *world = (World_t *)metaData;
	vec2_t grav = world->level->collisionWorld->gravity;
	world->level->character->sprite->flippedHorizontally = grav.y < 0.0f;
	CollisionPolyObject_t *collObj = world->level->character->collisionObject;
	if(world->level->character->collisionObject->inContact) {
		world->level->character->sprite->activeAnimation = aState == kInputState_down ? 6 : 7;
		collObj->velocity.x = -280.0f;
	} else
		collObj->velocity.x -= 20.0f;

}

static void characterCollided(CollisionWorld_t *aWorld, Collision_t collisionInfo)
{
	Character_t *character = (Character_t *)collisionInfo.objectA->info;
	if(character->sprite->activeAnimation == 5) {
		character->sprite->animations[4].currentFrame = 0;
		character->sprite->activeAnimation = 4;
	}
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
	if(out->level->character) out->level->character->collisionObject->collisionCallback = characterCollided;
	if(out->level->bgm) sound_play(out->level->bgm);
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
	aTimeDelta = MIN(aTimeDelta, 1.0f); // Cap the max collision timestep

	aWorld->time += aTimeDelta;
	++aWorld->ticks;

	if(aWorld->level->character) {
		if(aWorld->ticks % 2 == 0)
			sprite_step(aWorld->level->character->sprite);

		int timeSteps = 8;
		for(int i = 0; i < timeSteps; ++i)
			collision_step(aWorld->level->collisionWorld, aWorld->level->character->collisionObject, aTimeDelta/(double)timeSteps);
	}
}
