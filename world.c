#include "world.h"
#include "engine/sprite.h"
#include "engine/tmx_map.h"

static Sprite_t *sprite;
static bool charInContactWithGround = true;
static CollisionPolyObject_t *characterApprox;

static void downKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData)
{
	sprite->angle += M_PI/20;
	if(charInContactWithGround) {
		gWorld->collisionWorld->gravity.y *= -1.0f;
		charInContactWithGround = false;
	}
}
static void upKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData)
{
	static float scaleIncrementor = 0.0f;
	scaleIncrementor += M_PI/20;
	sprite->scale = 1.0f + 3.0*sinf(scaleIncrementor);

	if(charInContactWithGround || fabs(characterApprox->velocity.y) < 4.0f) {
		characterApprox->velocity.y = -1.0f*gWorld->collisionWorld->gravity.y*0.8;
//		characterApprox->velocity.y = MAX(characterApprox->velocity.x, 300.0f);	
		charInContactWithGround = false;
	}
}

static void rightKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData)
{
	World_t *world = (World_t *)metaData;
	sprite->flippedHorizontally = false;
	if(world->ticks%2 == 0) sprite_step(sprite);

	world->background->offset.x += 6.0;
	world->background->offset.y = floorf(8.0f+10.0f* sinf((float)world->ticks*M_PI/50.0f));
	sprite->location.y = floorf(41.0f-world->background->offset.y);
	if(charInContactWithGround && characterApprox->inContact)
		characterApprox->velocity.x = MAX(characterApprox->velocity.x, 300.0f);
	else if(!characterApprox->inContact)
		characterApprox->velocity.x += 5.0f;

	world->level->background->offset.x += 4.0;	
}
static void leftKeyPressed(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData)
{
	World_t *world = (World_t *)metaData;
	sprite->flippedHorizontally = true;
	if(world->ticks%2 == 0) sprite_step(sprite);

	world->background->offset.x -= 6.0;
	world->background->offset.y = floorf(8.0f+10.0f* cosf((float)world->ticks*M_PI/50.0f));
	sprite->location.y = floorf(41.0f-world->background->offset.y);

	if(charInContactWithGround && characterApprox->inContact)
		characterApprox->velocity.x = MIN(characterApprox->velocity.x, -300.0f);
	else if(!characterApprox->inContact)
		characterApprox->velocity.x -= 5.0f;

	world->level->background->offset.x -= 4.0;	
}

static void characterCollided(CollisionWorld_t *aWorld, Collision_t collisionInfo)
{
	if(fabs(collisionInfo.direction.y) > M_PI/4) {
		/*debug_log("Touched ground");*/
		charInContactWithGround = true;
	} else
		charInContactWithGround = false;
	//debug_log("CHARACTER COLLIDED!!");
	//printf("Str: %.2f: ",collisionInfo.magnitude); printVec2(collisionInfo.direction);
}

static void worldCollisionHappened(CollisionWorld_t *aWorld, Collision_t collisionInfo)
{
	/*debug_log("SOMETHING COLLIDED!!");*/
}

static void mouseMoved(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *metaData)
{
	static vec2_t lastGravity;
	static bool inDrag = false;
	World_t *world = (World_t *)metaData;
	static vec2_t lastLoc;
	static bool shouldResetDelta = true;
	vec2_t delta = kVec2_zero;
	if(!shouldResetDelta) delta = vec2_sub(*aLocation, lastLoc);

	world->background->offset = vec2_add(world->background->offset, delta);

	collision_setPolyObjectCenter(characterApprox, vec2_add(characterApprox->center, delta));
	if(aState == kInputState_up) {
		gWorld->collisionWorld->gravity = lastGravity;
		inDrag = false;
	}
	else if(aState == kInputState_down) {
		characterApprox->velocity = kVec2_zero;
		if(!inDrag)
			lastGravity = gWorld->collisionWorld->gravity;
		inDrag = true;
		gWorld->collisionWorld->gravity = kVec2_zero;
		//characterApprox->velocity = vec2_scalarMul(delta, 98.0f);
	}

	shouldResetDelta = (aState == kInputState_up);
	lastLoc = *aLocation;
}

World_t *world_init()
{
	World_t *out = malloc(sizeof(World_t));
	out->time = 0.0;
	out->ticks = 0;

	out->background = background_create();
	//out->background->layers[0] = background_createLayer(texture_loadFromPng("textures/backgrounds/clouds.png", true, false), 0.5);
	//out->background->layers[1] = background_createLayer(texture_loadFromPng("textures/backgrounds/hills.png", true, false), 0.3);
	//out->background->layers[2] = background_createLayer(texture_loadFromPng("textures/backgrounds/castle.png", true, false), 0.1);
	//out->background->layers[3] = background_createLayer(texture_loadFromPng("textures/backgrounds/ground.png", true, false), 0.0);
	out->background->layers[0] = background_createLayer(texture_loadFromPng("textures/backgrounds/stars1.png", true, true), 1.0);
	out->background->layers[1] = background_createLayer(texture_loadFromPng("textures/backgrounds/stars2.png", true, true), 0.7);
	out->background->layers[2] = background_createLayer(texture_loadFromPng("textures/backgrounds/stars3.png", true, true), 0.4);
	out->background->layers[3] = background_createLayer(texture_loadFromPng("textures/backgrounds/stars4.png", true, true), 0.2);
	//renderer_pushRenderable(gRenderer, &out->background->renderable);

	vec3_t spriteLoc = { 100.0f, 41.0f, 0.0f };
	vec2_t spriteSize = { 48.0f, 48.0f };
	TextureAtlas_t *atlas = texAtlas_create(texture_loadFromPng("textures/sonic.png", false, false), kVec2_zero, spriteSize);
	sprite = sprite_create(spriteLoc, spriteSize, atlas, 1);
	sprite->animations[0] = sprite_createAnimation(11);
	//renderer_pushRenderable(gRenderer, &sprite->renderable);

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


	// Create a collision world and populate with a couple of debug objects
	out->collisionWorld = collision_createWorld(vec2_create(0.0f, -980.0f), vec2_create(800, 632), 32);
	//renderer_pushRenderable(gRenderer, &out->collisionWorld->debugRenderable);

	vec2_t rectVerts[4];
	rectVerts[0] = vec2_create(340, 300);
	rectVerts[1] = vec2_create(340, 320);
	rectVerts[2] = vec2_create(350, 320);
	rectVerts[3] = vec2_create(350, 300);

	CollisionPolyObject_t *rectangle1 = collision_createPolyObject(4, rectVerts, 0.2, 1.0);
	spatialHash_addItem(out->collisionWorld->spatialHash, rectangle1, rectangle1->boundingBox);

	rectVerts[0] = vec2_create(350, 290);
	rectVerts[1] = vec2_create(350, 300);
	rectVerts[2] = vec2_create(400, 300);
	rectVerts[3] = vec2_create(400, 290);

	CollisionPolyObject_t *rectangle2 = collision_createPolyObject(4, rectVerts, 0.5, 0.1);
	spatialHash_addItem(out->collisionWorld->spatialHash, rectangle2, rectangle2->boundingBox);

	rectVerts[0] = vec2_create(0, 0);
	rectVerts[1] = vec2_create(0, 20);
	rectVerts[2] = vec2_create(800, 20);
	rectVerts[3] = vec2_create(800, 0);

	CollisionPolyObject_t *rectangle3 = collision_createPolyObject(4, rectVerts, 0.1, 0.5);
	spatialHash_addItem(out->collisionWorld->spatialHash, rectangle3, rectangle3->boundingBox);
	
	rectVerts[0] = vec2_create(0, 580);
	rectVerts[1] = vec2_create(0, 600);
	rectVerts[2] = vec2_create(800, 600);
	rectVerts[3] = vec2_create(800, 580);

	CollisionPolyObject_t *rectangle4 = collision_createPolyObject(4, rectVerts, 0.5, 0.2);
	spatialHash_addItem(out->collisionWorld->spatialHash, rectangle4, rectangle4->boundingBox);

	rectVerts[0] = vec2_create(0, 0);
	rectVerts[1] = vec2_create(0, 600);
	rectVerts[2] = vec2_create(20, 600);
	rectVerts[3] = vec2_create(20, 0);

	CollisionPolyObject_t *rectangle5 = collision_createPolyObject(4, rectVerts, 0.5, 0.2);
	spatialHash_addItem(out->collisionWorld->spatialHash, rectangle5, rectangle5->boundingBox);

	rectVerts[0] = vec2_create(780, 0);
	rectVerts[1] = vec2_create(780, 600);
	rectVerts[2] = vec2_create(800, 600);
	rectVerts[3] = vec2_create(800, 0);

	CollisionPolyObject_t *rectangle6 = collision_createPolyObject(4, rectVerts, 0.2, 0.9);
	spatialHash_addItem(out->collisionWorld->spatialHash, rectangle6, rectangle6->boundingBox);


	vec2_t triVerts[3];
	triVerts[0] = vec2_create(400, 300);
	triVerts[1] = vec2_create(455, 330);
	triVerts[2] = vec2_create(455, 300);

	CollisionPolyObject_t *triangle1 = collision_createPolyObject(3, triVerts, 0.4, 0.3);
	spatialHash_addItem(out->collisionWorld->spatialHash, triangle1, triangle1->boundingBox);

	triVerts[0] = vec2_create(0, 0);
	triVerts[1] = vec2_create(0, 200);
	triVerts[2] = vec2_create(333, 0);


	CollisionPolyObject_t *triangle2 = collision_createPolyObject(3, triVerts, 0.06, 0.3);
	spatialHash_addItem(out->collisionWorld->spatialHash, triangle2, triangle2->boundingBox);


	rectVerts[0] = vec2_create(0, 0);
	rectVerts[1] = vec2_create(0, 32);
	rectVerts[2] = vec2_create(32, 32);
	rectVerts[3] = vec2_create(32, 0);

	characterApprox = collision_createPolyObject(4, rectVerts, 0.0, 0.0);
//	characterApprox->angle = M_PI/6.0f;
	//characterApprox->orientation = quat_makef(0.0f, 0.0f, 1.0f, M_PI/6.0f);
	collision_setPolyObjectCenter(characterApprox, vec2_create(410, 528));
	//characterApprox->angularVelocity = M_PI/7.0f;

	out->collisionWorld->character = characterApprox;
	characterApprox->collisionCallback = &characterCollided;
	out->collisionWorld->collisionCallback = &worldCollisionHappened;
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
	
	//aWorld->collisionWorld->character->velocity.y -= 300.0f*aTimeDelta;
	aWorld->collisionWorld->character->velocity = vec2_add(aWorld->collisionWorld->character->velocity, vec2_scalarMul(gWorld->collisionWorld->gravity, aTimeDelta));
	
	aWorld->level->background->offset.x += 1.0;
	aWorld->level->background->offset.y = 5.0*sinf(gGameTimer.elapsed);	

	int timeSteps = 8;
	for(int i = 0; i < timeSteps; ++i)
		collision_step(aWorld->collisionWorld, aWorld->collisionWorld->character, aTimeDelta/(float)timeSteps);
}
