#include "gametimer.h"
#include "util.h"

#ifdef __APPLE__
#include <mach/mach_time.h>
#endif

#ifdef ANDROID
#include <time.h>
#endif

struct ScheduledCallbackWrapper_t {
    GLMFloat time;
    GameTimer_scheduledCallback_t callback;
    void *context;
};

static void _callScheduledCallbackIfNeeded(struct ScheduledCallbackWrapper_t *aWrapper, GameTimer_t *aTimer);
static void gameTimer_destroy(GameTimer_t *aGameTimer);

Class_t Class_GameTimer = {
	"GameTimer",
	sizeof(GameTimer_t),
	(Obj_destructor_t)&gameTimer_destroy
};

GameTimer_t *gameTimer_create(GLMFloat aFps, GameTimer_updateCallback_t aUpdateCallback)
{
	GameTimer_t *out = obj_create_autoreleased(&Class_GameTimer);
	out->desiredInterval = 1.0/(aFps > 0.0 ? aFps : 60.0);
	out->updateCallback = aUpdateCallback;
    out->scheduledCallbacks = obj_retain(llist_create(NULL, &free));

	return out;
}
void gameTimer_destroy(GameTimer_t *aTimer)
{
    obj_release(aTimer->scheduledCallbacks);
}

extern void gameTimer_step(GameTimer_t *aTimer, GLMFloat aElapsed)
{
	GLMFloat delta = aElapsed - aTimer->elapsed;
	aTimer->timeSinceLastUpdate = MAX(0.0, aTimer->timeSinceLastUpdate+delta);
	aTimer->elapsed = aElapsed;
    
    // Execute any scheduled callbacks
    llist_apply(aTimer->scheduledCallbacks, (LinkedListApplier_t)&_callScheduledCallbackIfNeeded, aTimer);
    
	for(; aTimer->timeSinceLastUpdate > aTimer->desiredInterval; aTimer->timeSinceLastUpdate -= aTimer->desiredInterval) {
		if(aTimer->updateCallback)
			aTimer->updateCallback(aTimer);
		++aTimer->ticks;
	}
}

GLMFloat gameTimer_interpolationSinceLastUpdate(GameTimer_t *aTimer)
{
	return aTimer->timeSinceLastUpdate / aTimer->desiredInterval;
}

#pragma mark - Scheduled callbacks

void gameTimer_afterDelay(GameTimer_t *aTimer, GLMFloat aDelay, GameTimer_scheduledCallback_t aCallback, void *aContext)
{
    dynamo_assert(aCallback != NULL, "Invalid callback");
    struct ScheduledCallbackWrapper_t *wrapper = malloc(sizeof(struct ScheduledCallbackWrapper_t));
    wrapper->time = dynamo_time() + aDelay;
    wrapper->callback = aCallback;
    wrapper->context = aContext;
    llist_pushValue(aTimer->scheduledCallbacks, wrapper);
}

static void _callScheduledCallbackIfNeeded(struct ScheduledCallbackWrapper_t *aWrapper, GameTimer_t *aTimer)
{
    if(aWrapper->time > aTimer->elapsed)
        return;
    aWrapper->callback(aTimer, aWrapper->context);
    llist_deleteValue(aTimer->scheduledCallbacks, aWrapper);
}


#pragma mark - General time functions

GLMFloat dynamo_globalTime()
{
    GLMFloat ret = -1.0;
#ifdef __APPLE__
    mach_timebase_info_data_t timebase;
    mach_timebase_info(&timebase);
    uint64_t absolute = mach_absolute_time();
    GLMFloat nanoSecs = absolute * timebase.numer / timebase.denom;
    ret = nanoSecs / NSEC_PER_SEC;
#elif defined(ANDROID)
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);
    ret = now.tv_sec + now.tv_nsec/1000000000.0;
#else
    #error "Time functions not yet implemented for this platform"
#endif
    return ret;
}

GLMFloat dynamo_time()
{
    static GLMFloat startTime = -1.0;
    if(startTime < -1.0)
        startTime = dynamo_globalTime();
    return dynamo_globalTime() - startTime;
}
