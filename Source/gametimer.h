/*!
    @header Drawing Utilities
    @abstract
    @discussion Manages the game loop.
*/

#include "object.h"
#include <stdbool.h>
#include <GLMath/GLMath.h>

#ifndef _GAMETIMER_H_
#define _GAMETIMER_H_

extern Class_t Class_GameTimer;

typedef struct _GameTimer GameTimer_t;
/*!
    A callback that is called on every update of the game.
*/
typedef void (*GameTimer_updateCallback_t)(GameTimer_t *aTimer);
/*!
    A callback that is called after a specified duration.
*/
typedef void (*GameTimer_scheduledCallbackInvoke_t)(GameTimer_t *aTimer, void *aContext);

typedef struct _GameTimer_ScheduledCallback {
    GLMFloat time; // Interpreted as an interval if the callback repeats
    bool repeats;
    GameTimer_scheduledCallbackInvoke_t callback;
    int luaCallback;
    void *context;
    GLMFloat lastFired;
} GameTimer_ScheduledCallback_t;

/*!
    A game timer
    (All values are in seconds)

    @field elapsed The elapsed time
    @field timeSinceLastUpdate Time since the last game update
    @field desiredInterval The desired interval between game updates
    @field ticks The number of game cycles since startup
    @field updateCallback A function that is called on every iteration of the game loop
*/
struct _GameTimer {
    OBJ_GUTS
    GLMFloat elapsed;
    GLMFloat timeSinceLastUpdate;
    GLMFloat desiredInterval; // The minimum interval between updates
    GLMFloat resetAt; // This value is subtracted from the elapsed time
    long ticks;
    GameTimer_updateCallback_t updateCallback;
    int luaUpdateCallback;
    LinkedList_t *scheduledCallbacks;    
};

/*!
    Creates a game timer.
*/
extern GameTimer_t *gameTimer_create(GLMFloat aFps, GameTimer_updateCallback_t aUpdateCallback);

/*!
    Updates the timer and calls the update callback as many times as required to progress up until elapsed.
*/
extern void gameTimer_step(GameTimer_t *aTimer, GLMFloat elapsed);
/*!
    Returns a value from 0-1 indicating the current interpolation between game updates.
*/
extern GLMFloat gameTimer_interpolationSinceLastUpdate(GameTimer_t *aTimer);
/*!
    Resets the timer to 0.
*/
extern void gameTimer_reset(GameTimer_t *aTimer);
/*!
    Executes the given callback after a given delay.
*/
extern GameTimer_ScheduledCallback_t *gameTimer_afterDelay(GameTimer_t *aTimer, GLMFloat aDelay, GameTimer_scheduledCallbackInvoke_t aCallback, bool aRepeats, void *aContext);
/*!
    Executes the given Lua callback after a given delay.
*/
extern GameTimer_ScheduledCallback_t *gameTimer_afterDelay_luaCallback(GameTimer_t *aTimer, GLMFloat aDelay, int aCallback, bool aRepeats);
/*
    Unschedules a callback.
*/
extern bool gameTimer_unscheduleCallback(GameTimer_t *aTimer, GameTimer_ScheduledCallback_t *aCallback);

// Returns the time in seconds
extern GLMFloat dynamo_globalTime();
// Returns the duration since the application launch in seconds
extern GLMFloat dynamo_time();
#endif
