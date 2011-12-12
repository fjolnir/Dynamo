// The game world
// Handles the state of the game

#include "shared.h"
#include "engine/background.h"
#include "engine/input.h"
#include "level.h"
#include "collision.h"

#ifndef _WORLD_H_
#define _WORLD_H_

typedef struct _World World_t;

struct _World {
	double time; // Game time in seconds
	long ticks; // Number of game updates since beginning (roughly time/FPS)
	Background_t *background;

	Level_t *level;
	CollisionWorld_t *collisionWorld;

	vec2_t gravity;

	InputObserver_t *arrowRightObserver;
	InputObserver_t *arrowLeftObserver;
	InputObserver_t *arrowUpObserver;
	InputObserver_t *arrowDownObserver;
	InputObserver_t *leftClickObserver;
	InputObserver_t *leftDragObserver;
};

extern World_t *world_init();
extern void world_destroy(World_t *aWorld);
void world_update(World_t *aWorld, double aTimeDelta);
#endif
