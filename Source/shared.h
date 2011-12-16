#ifndef _SHARED_H_
#define _SHARED_H_

#include "engine/glutils.h"
#include "engine/GLMath/GLMath.h"
#include "engine/shader.h"
#include "engine/renderer.h"
#include "engine/gametimer.h"
#include "engine/input.h"
#include "engine/various.h"
#include "engine/sound.h"
#include "engine/drawutils.h"

#include "world.h"

#pragma mark - Constants

#define DESIRED_FPS 30 // The update interval. Actual screen updates are done as fast as possible.
#define MAX_FRAMESKIP 5

#pragma mark - Globals

// The main renderer object (Defined in main.c)
extern Renderer_t *gRenderer;
// The game world
extern World_t *gWorld;
// The game timer
extern GameTimer_t gGameTimer;
// The input manager
extern InputManager_t *gInputManager;
// The sound manager
extern SoundManager_t *gSoundManager;

// Quits the game
extern void quitGame();

#endif
