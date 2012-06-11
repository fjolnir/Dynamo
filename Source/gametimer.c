#include "gametimer.h"
#include "util.h"
#include "luacontext.h"

#ifdef __APPLE__
#include <mach/mach_time.h>
#endif

#ifdef ANDROID
#include <time.h>
#endif

struct ScheduledCallbackWrapper_t {
    GLMFloat time;
    GameTimer_scheduledCallback_t callback;
    int luaCallback;
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
    out->luaUpdateCallback = -1;

	return out;
}
void gameTimer_destroy(GameTimer_t *aTimer)
{
    if(aTimer->luaUpdateCallback != -1)
        luaCtx_unregisterScriptHandler(GlobalLuaContext, aTimer->luaUpdateCallback);
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
        if(aTimer->luaUpdateCallback != -1) {
            printf("foo\n");
            luaCtx_pushScriptHandler(GlobalLuaContext, aTimer->luaUpdateCallback);
            luaCtx_pushnumber(GlobalLuaContext, aTimer->ticks);
            luaCtx_pushnumber(GlobalLuaContext, aTimer->elapsed);
            luaCtx_pushnumber(GlobalLuaContext, aTimer->timeSinceLastUpdate);
            luaCtx_pushnumber(GlobalLuaContext, aTimer->desiredInterval);
            luaCtx_pcall(GlobalLuaContext, 4, 0, 0);
        }
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
    wrapper->luaCallback = -1;
    wrapper->context = aContext;
    llist_pushValue(aTimer->scheduledCallbacks, wrapper);
}

void gameTimer_afterDelay_luaCallback(GameTimer_t *aTimer, GLMFloat aDelay, int aCallback)
{
    dynamo_assert(aCallback != -1, "Invalid callback");
    struct ScheduledCallbackWrapper_t *wrapper = malloc(sizeof(struct ScheduledCallbackWrapper_t));
    wrapper->time = dynamo_time() + aDelay;
    wrapper->luaCallback = aCallback;
    wrapper->context = NULL;
    llist_pushValue(aTimer->scheduledCallbacks, wrapper);
}

static void _callScheduledCallbackIfNeeded(struct ScheduledCallbackWrapper_t *aWrapper, GameTimer_t *aTimer)
{
    if(aWrapper->time > aTimer->elapsed)
        return;
    if(aWrapper->callback)
        aWrapper->callback(aTimer, aWrapper->context);
    if(aWrapper->luaCallback != -1) {
        luaCtx_pushScriptHandler(GlobalLuaContext, aWrapper->luaCallback);
        luaCtx_pcall(GlobalLuaContext, 0, 0, 0);
    }
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
#elif defined(ANDROID) // This should work on any linux distro. Verify.
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
    if(startTime <= -1.0)
        startTime = dynamo_globalTime();
    return dynamo_globalTime() - startTime;
}
