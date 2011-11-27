// The game world
// Handles the state of the game

#include "shared.h"
#include "engine/background.h"
#include "engine/input.h"
#include "level.h"

#ifndef _WORLD_H_
#define _WORLD_H_

typedef struct _World {
	double time; // Game time in seconds
	long ticks; // Number of game updates since beginning (roughly time/FPS)
	Background_t *background;

	Level_t *level;

	InputObserver_t *arrowRightObserver;
	InputObserver_t *arrowLeftObserver;
	InputObserver_t *arrowUpObserver;
	InputObserver_t *arrowDownObserver;
	InputObserver_t *leftClickObserver;
	InputObserver_t *leftDragObserver;
} World_t;

extern World_t *world_init();
extern void world_destroy(World_t *aWorld);
void world_update(World_t *aWorld, double aTimeDelta);
#endif
