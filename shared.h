#ifndef _SHARED_H_
#define _SHARED_H_

#include "utils/glutils.h"
#include "utils/GLMath/GLMath.h"
#include "utils/shader.h"
#include "utils/renderer.h"
#include "utils/gametimer.h"
#include "utils/input.h"
#include "utils/various.h"
#include "utils/sound.h"
#include "world.h"

#pragma mark - Constants

#define DESIRED_FPS 60 // The update interval. Actual screen updates are done as fast as possible.


#pragma mark - Globals

// The main renderer object (Defined in main.c)
extern Renderer_t *gRenderer;
// A basic texture mapping shader
extern Shader_t *gBasicShader;
// The game world
extern World_t *gWorld;
// The input manager
extern InputManager_t *gInputManager;
// The sound manager
extern SoundManager_t *gSoundManager;
#endif
