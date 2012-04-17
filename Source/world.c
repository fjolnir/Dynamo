#include "world.h"
#include "shared.h"
#include "engine/sprite.h"
#include "engine/tmx_map.h"

static void world_destroy(World_t *aWorld);

static bool _characterInContactWithGround = true;
static void downKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData);
static void upKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData);
static void rightKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData);
static void leftKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData);

static void characterCollided(CollisionWorld_t *aWorld, Collision_t collisionInfo);

static void mainMenu_selectionChanged(MainMenu_t *aMenu, int aSelection, void *metaData);
static void mainMenu_didFadeOut(MainMenu_t *aMenu, void *metaData);

static int upKeyHeldFrameCount = 0;

#pragma mark - Initialization

World_t *world_init()
{
	World_t *out = obj_create_autoreleased(sizeof(World_t), (Obj_destructor_t)&world_destroy);
	out->time = 0.0;
	out->ticks = 0;
	out->level = NULL;

		
	// Create&add input observers
	out->arrowRightObserver = input_createObserver(kInputKey_arrowRight, &rightKeyPressed, NULL, out);
	out->arrowLeftObserver = input_createObserver(kInputKey_arrowLeft, &leftKeyPressed, NULL, out);
	out->arrowUpObserver = input_createObserver(kInputKey_arrowUp, &upKeyPressed, NULL, out);
	out->arrowDownObserver = input_createObserver(kInputKey_arrowDown, &downKeyPressed, NULL, out);

	input_addObserver(gInputManager, out->arrowRightObserver);
	input_addObserver(gInputManager, out->arrowLeftObserver);
	input_addObserver(gInputManager, out->arrowUpObserver);
	input_addObserver(gInputManager, out->arrowDownObserver);

	out->menu = obj_retain(mainMenu_create());
	out->menu->metaData = out;
	out->menu->selectionCallback = &mainMenu_selectionChanged;
	out->menu->fadeCallback = &mainMenu_didFadeOut;
	renderer_pushRenderable(gRenderer, &out->menu->renderable);


	return out;
}

void world_destroy(World_t *aWorld)
{
	if(aWorld->menu) obj_release(aWorld->menu);
	aWorld->menu = NULL;
	if(aWorld->level) obj_release(aWorld->level);
	aWorld->level = NULL;
	input_removeObserver(gInputManager, aWorld->arrowRightObserver);
	obj_release(aWorld->arrowRightObserver);
	aWorld->arrowRightObserver = NULL;
	input_removeObserver(gInputManager, aWorld->arrowLeftObserver);
	obj_release(aWorld->arrowLeftObserver);
	aWorld->arrowLeftObserver = NULL;
	input_removeObserver(gInputManager, aWorld->arrowUpObserver);
	obj_release(aWorld->arrowUpObserver);
	aWorld->arrowUpObserver = NULL;
	input_removeObserver(gInputManager, aWorld->arrowDownObserver);
	obj_release(aWorld->arrowDownObserver);
	aWorld->arrowDownObserver = NULL;
}


#pragma mark - Game logic

void world_update(World_t *aWorld, double aTimeDelta)
{
	aTimeDelta = MIN(aTimeDelta, 1.0f); // Cap the max collision timestep

	aWorld->time += aTimeDelta;
	++aWorld->ticks;

	if(aWorld->menu) mainMenu_update(aWorld->menu, aTimeDelta);

	if(!aWorld->level || !aWorld->level->character) return;

	// Make the character stick his hands out when he's been sliding for a few frames
	CollisionPolyObject_t *collisionObj = aWorld->level->character->collisionObject;
	static int slideCounter = 0;
	bool running = aWorld->arrowRightObserver->lastKnownState == kInputState_down
		|| aWorld->arrowLeftObserver->lastKnownState == kInputState_down;
	Character_t *character = aWorld->level->character;
	if(!running && collisionObj->inContact && character->sprite->activeAnimation != 5) {
		if(fabs(collisionObj->velocity.x) >= 10.0f || fabs(collisionObj->velocity.y) >= 10.0) {
			if(slideCounter >= 5) character->sprite->activeAnimation = 1;
			++slideCounter;
		} else {
			character->sprite->activeAnimation = 7;
			slideCounter = 0;
		}
	}

	// Animate the sprite
	if(aWorld->ticks % 2 == 0)
		sprite_step(aWorld->level->character->sprite);

	// Step the collision world
	int timeSteps = 8;
	for(int i = 0; i < timeSteps; ++i)
		collision_step(aWorld->level->collisionWorld, aWorld->level->character->collisionObject, aTimeDelta/(double)timeSteps);
}

static void characterCollided(CollisionWorld_t *aWorld, Collision_t collisionInfo)
{
	Character_t *character = (Character_t *)collisionInfo.objectA->info;

	_characterInContactWithGround = vec2_dot(aWorld->gravity, collisionInfo.direction) < 0.0f;

	if(character->sprite->activeAnimation == 5 && _characterInContactWithGround) {
		character->sprite->animations[4].currentFrame = 0;
		character->sprite->activeAnimation = 4;
		upKeyHeldFrameCount = 0;
	}
}


#pragma mark - Input handling

static void downKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData)
{
	if(aState != kInputState_down) return;
	World_t *world = (World_t *)metaData;
	if(!world->level) return;
	Character_t *character = world->level->character;
	if(character->collisionObject->inContact && _characterInContactWithGround) {
		_characterInContactWithGround = false;
		character->sprite->animations[5].currentFrame = 0;
		character->sprite->activeAnimation = 5;
		world->level->collisionWorld->gravity.y *= -1.0f;
	}
}
static void upKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData)
{
	if(aState != kInputState_down) {
		upKeyHeldFrameCount = 0;
		return;
	}

	World_t *world = (World_t *)metaData;
	if(!world->level) return;
	Character_t *character = world->level->character;
	CollisionPolyObject_t *collObj = character->collisionObject;
	if(character->collisionObject->inContact && upKeyHeldFrameCount == 0) {
		collObj->velocity.y = -0.3f * world->level->collisionWorld->gravity.y;
		character->sprite->animations[5].currentFrame = 0;
		character->sprite->activeAnimation = 5;
		_characterInContactWithGround = false;
	} else if(upKeyHeldFrameCount > 0 && upKeyHeldFrameCount < 15)
		collObj->velocity.y -= 0.025f * world->level->collisionWorld->gravity.y;

	++upKeyHeldFrameCount;
}

static void rightKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData)
{
	World_t *world = (World_t *)metaData;
	if(!world->level) return;
	vec2_t grav = world->level->collisionWorld->gravity;
	CollisionPolyObject_t *collObj = world->level->character->collisionObject;
	if(aState != kInputState_down && !_characterInContactWithGround) {
		collObj->velocity.x *= 0.3f;
	} else if(_characterInContactWithGround) {
		world->level->character->sprite->flippedHorizontally = grav.y > 0.0f;
		world->level->character->sprite->activeAnimation = aState == kInputState_down ? 6 : 7;
		collObj->velocity.x = 280.0f;
	} else
		collObj->velocity.x += 10.0f;
}

static void leftKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData)
{
	World_t *world = (World_t *)metaData;
	vec2_t grav = world->level->collisionWorld->gravity;
	CollisionPolyObject_t *collObj = world->level->character->collisionObject;
	if(aState != kInputState_down && !_characterInContactWithGround) {
		collObj->velocity.x *= 0.3f;
	} else if(_characterInContactWithGround) {
		world->level->character->sprite->flippedHorizontally = grav.y < 0.0f;
		world->level->character->sprite->activeAnimation = aState == kInputState_down ? 6 : 7;
		collObj->velocity.x = -280.0f;
	} else
		collObj->velocity.x -= 10.0f;
}

static void mainMenu_selectionChanged(MainMenu_t *aMenu, int aSelection, void *metaData)
{
	aMenu->disabled = true;
	mainMenu_fadeOut(aMenu);
}

static void mainMenu_didFadeOut(MainMenu_t *aMenu, void *metaData)
{
	World_t *world = (World_t *)metaData;
	int selection = aMenu->selectedItem;
	if(selection == 0)
		quitGame();
	else if(selection == 1) {
		world->level = obj_retain(level_load("levels/spacetest.tmx"));
		if(world->level->character) world->level->character->collisionObject->collisionCallback = characterCollided;
		if(world->level->bgm) sound_play(world->level->bgm);
		renderer_popRenderable(gRenderer); // Remove the menu from the render stack
		renderer_pushRenderable(gRenderer, &world->level->renderable);
	}
}

