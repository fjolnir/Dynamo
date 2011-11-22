// The game world
// Handles the state of the game

#include "shared.h"
#include "engine/background.h"
#include "engine/input.h"

#ifndef _WORLD_H_
#define _WORLD_H_

typedef struct _World {
	long time;
	Background_t *background;
	InputObserver_t *arrowRightObserver;
	InputObserver_t *arrowLeftObserver;
	InputObserver_t *arrowUpObserver;
	InputObserver_t *arrowDownObserver;
	InputObserver_t *leftClickObserver;
	InputObserver_t *leftDragObserver;
} World_t;

extern World_t *world_init();
extern void world_destroy(World_t *aWorld);
extern void world_update(World_t *aWorld);
#endif
