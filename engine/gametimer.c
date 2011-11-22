#include "gametimer.h"
#include <float.h>
#include "various.h"

void gameTimer_update(GameTimer_t *aTimer, double aTime)
{
	double delta = aTime - aTimer->elapsed;
//	printf("%.2f - %.2f = %.2f (%.2f: %d)", aTimer->elapsed, aTime, delta, aTimer->desiredInterval, delta>aTimer->desiredInterval);
	aTimer->elapsed = aTime;
	aTimer->timeSinceLastUpdate = MAX(0, aTimer->timeSinceLastUpdate+delta);
//	printf(" TIME SINCE LAST: %.2f\n", aTimer->timeSinceLastUpdate);

	
	if(delta < FLT_EPSILON) return; // If the difference is negligible just go on 

	// Otherwise, calculate the FPS
	aTimer->estimatedFPS = 0.9*aTimer->estimatedFPS + (0.1*1000.0)/delta;
}
