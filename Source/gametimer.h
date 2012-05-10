#include "object.h"
#include <stdbool.h>
#include <GLMath/GLMath.h>

#ifndef _GAMETIMER_H_
#define _GAMETIMER_H_

extern Class_t Class_GameTimer;

typedef struct _GameTimer GameTimer_t;
typedef void (*GameTimer_updateCallback_t)(GameTimer_t *aTimer);
typedef void (*GameTimer_scheduledCallback_t)(GameTimer_t *aTimer, void *aContext);

// All values are in seconds
struct _GameTimer {
	OBJ_GUTS
	GLMFloat elapsed;
	GLMFloat timeSinceLastUpdate;
	GLMFloat desiredInterval; // The minimum interval between updates
	long ticks;
	GameTimer_updateCallback_t updateCallback;
    LinkedList_t *scheduledCallbacks;
};

extern GameTimer_t *gameTimer_create(GLMFloat aFps, GameTimer_updateCallback_t aUpdateCallback);

// Updates the timer and calls the update callback as many times as required to progress up until elapsed
extern void gameTimer_step(GameTimer_t *aTimer, GLMFloat elapsed);

extern GLMFloat gameTimer_interpolationSinceLastUpdate(GameTimer_t *aTimer);

extern void gameTimer_afterDelay(GameTimer_t *aTimer, GLMFloat aDelay, GameTimer_scheduledCallback_t aCallback, void *aContext);
#endif
