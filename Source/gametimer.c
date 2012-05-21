#include "gametimer.h"
#include "util.h"

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
    wrapper->time = aTimer->elapsed + aDelay;
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