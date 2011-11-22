#include <stdbool.h>

#ifndef _GAMETIMER_H_
#define _GAMETIMER_H_

typedef struct _GameTimer {
	double elapsed;
	double timeSinceLastUpdate;
	double desiredInterval; // The minimum interval between updates
	double estimatedFPS;
} GameTimer_t;

extern void gameTimer_update(GameTimer_t *aTimer, double aTime);
extern void gameTimer_step(GameTimer_t *aTimer);
extern bool gameTimer_reachedNextFrame(GameTimer_t *aTimer);

#endif
