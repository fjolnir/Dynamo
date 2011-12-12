#include <stdbool.h>

#ifndef _GAMETIMER_H_
#define _GAMETIMER_H_

// All values are in seconds
typedef struct _GameTimer {
	double elapsed;
	double timeSinceLastUpdate;
	double desiredInterval; // The minimum interval between updates

	double lastAbsoluteTime;
} GameTimer_t;

extern void gameTimer_update(GameTimer_t *aTimer, double aDelta);
extern void gameTimer_finishedUpdate(GameTimer_t *aTimer);
extern bool gameTimer_reachedNextUpdate(GameTimer_t *aTimer);
extern double gameTimer_interpolationSinceLastUpdate(GameTimer_t *aTimer);
#endif
