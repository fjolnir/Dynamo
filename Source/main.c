// Just handles the runloop

#ifdef WIN32
	#include <windows.h>
#endif

#include <stdio.h>
#include "engine/glutils.h"
#include "shared.h"
#include <SDL/SDL.h>

// Constants
#define MSEC_PER_SEC (1000)

// Globals
Renderer_t *gRenderer;
World_t *gWorld;
GameTimer_t gGameTimer;
InputManager_t *gInputManager;
SoundManager_t *gSoundManager;

// Locals
static bool _leftMouseButtonDown, _rightMouseButtonDown;
static Uint32 lastTime;
static bool _shouldExit = false;
static SDL_Surface *_sdlSurface;


// Functions
static void cleanup();

#pragma mark - Drawing


static void windowDidResize(int aWidth, int aHeight)
{
	gRenderer->viewportSize.w = (float)aWidth;
	gRenderer->viewportSize.h = (float)aHeight;

	matrix_stack_pop(gRenderer->projectionMatrixStack);
	matrix_stack_push_item(gRenderer->projectionMatrixStack, mat4_ortho(0.0f, (float)aWidth, 0.0f, (float)aHeight, -1.0f, 1.0f));
}


#pragma mark - Event handling

static Input_type_t _inputTypeForSDLKey(SDLKey aKey)
{
	switch(aKey) {
		case SDLK_LEFT:
			return kInputKey_arrowLeft;
		case SDLK_RIGHT:
			return kInputKey_arrowRight;
		case SDLK_UP:
			return kInputKey_arrowUp;
		case SDLK_DOWN:
			return kInputKey_arrowDown;
		default:
			return -1;
	}
}

static void handleEvent(SDL_Event aEvent)
{
	switch(aEvent.type) {
		case SDL_QUIT:
			_shouldExit = true;
			break;
		case SDL_ACTIVEEVENT:
		{
			if (aEvent.active.gain) {
				// Resume
			}
			else {
				 // Pause
			}
			break;
		}
		case SDL_VIDEORESIZE:
		{
			windowDidResize(aEvent.resize.w, aEvent.resize.h);
		}
		case SDL_KEYDOWN:
		{
			if(aEvent.key.keysym.sym == 'q')
				_shouldExit = true;
			if(aEvent.key.keysym.sym == SDLK_RETURN)
				input_beginEvent(gInputManager, kInputKey_ascii, (unsigned char*)"\n", NULL);
			else if(aEvent.key.keysym.sym < 127)
				input_beginEvent(gInputManager, kInputKey_ascii, (unsigned char*)&aEvent.key.keysym.sym, NULL);
			else {
				Input_type_t inputType = _inputTypeForSDLKey(aEvent.key.keysym.sym);
				if(inputType != -1)
					input_beginEvent(gInputManager, inputType, NULL, NULL);
			}
			break;
		}
		case SDL_KEYUP:
		{
			if(aEvent.key.keysym.sym == SDLK_RETURN)
				input_endEvent(gInputManager, kInputKey_ascii, (unsigned char*)"\n");
			else if(aEvent.key.keysym.sym < 127)
				input_endEvent(gInputManager, kInputKey_ascii, (unsigned char*)&aEvent.key.keysym.sym);
			else {
				Input_type_t inputType = _inputTypeForSDLKey(aEvent.key.keysym.sym);
				if(inputType == -1) break;
				input_endEvent(gInputManager, inputType, NULL);
			}
			break;
		}
		case SDL_MOUSEMOTION:
		{
			SDL_MouseMotionEvent motion = aEvent.motion;
			vec2_t location = { (float)motion.x, gRenderer->viewportSize.h - (float)motion.y };
			input_postMomentaryEvent(gInputManager, kInputMouse_move, NULL, &location, kInputState_up);

			if(_leftMouseButtonDown)
				input_postMomentaryEvent(gInputManager, kInputMouse_leftDrag, NULL, &location, kInputState_down);
			if(_rightMouseButtonDown)
				input_postMomentaryEvent(gInputManager, kInputMouse_rightDrag, NULL, &location, kInputState_down);
			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
		{
			SDL_MouseButtonEvent buttonEvent = aEvent.button;
			vec2_t location = { (float)buttonEvent.x, gRenderer->viewportSize.h - (float)buttonEvent.y };

			Input_type_t inputType;
			switch(buttonEvent.button) {
				case SDL_BUTTON_LEFT:
					_leftMouseButtonDown = (buttonEvent.state == SDL_PRESSED);
					inputType = kInputMouse_leftClick;
					break;
				case SDL_BUTTON_RIGHT:
					_rightMouseButtonDown = (buttonEvent.state == SDL_PRESSED);
					inputType = kInputMouse_rightClick;
					break;
				default:
					return;
			}
			input_postMomentaryEvent(gInputManager, inputType, NULL,
			                         &location, (buttonEvent.state == SDL_PRESSED) ? kInputState_down : kInputState_up);
			break;
		}
	}
}


#pragma mark - Initialization & Cleanup

void quitGame()
{
	_shouldExit = true;
}

static void cleanup()
{
	debug_log("Quitting...");
	if(gSoundManager) soundManager_destroy(gSoundManager);
	if(gWorld) world_destroy(gWorld);
	if(gRenderer) renderer_destroy(gRenderer);
	if(gInputManager) input_destroyManager(gInputManager);
	draw_cleanup();
	SDL_Quit();
}

int main(int argc, char **argv)
{
	#if defined(WIN32) && defined(TWODEEDENG_DEBUG)
	// Redirect back to console (SDL redirects standard outputs to log files by default on windows)
	freopen("CON", "w", stdout);
	freopen("CON", "w", stderr);
	#endif

	bool shouldFullscreen = (argc == 2 && strcmp(argv[1], "-f") == 0);

	vec2_t viewport = { 800.0f, 600.0f };

	// Initialize graphics
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0) {
		debug_log("Couldn't initialize SDL");
		return 1;
	}
	atexit(&cleanup);

	const SDL_VideoInfo *info = SDL_GetVideoInfo();
	if(!info) {
		debug_log("Failed to get information about graphics hardware");
		return 1;
	}
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1); // VSync

	int videoFlags = SDL_OPENGL;
	if(shouldFullscreen) {
		videoFlags |= SDL_FULLSCREEN;
		SDL_ShowCursor(SDL_DISABLE);
	}

	_sdlSurface = SDL_SetVideoMode(viewport.w, viewport.h, 16, videoFlags);
	if(!_sdlSurface) {
		debug_log("Couldn't initialize SDL surface");
		debug_log("%s", SDL_GetError());
		return 1;
	}

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glLineWidth(2.0);
	/*glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
*/
	gSoundManager = soundManager_create();

	gRenderer = renderer_create(viewport, kVec3_zero);
	draw_init(gRenderer);

	gGameTimer.desiredInterval = 1.0/(double)DESIRED_FPS;
	gGameTimer.elapsed = 0.0;
	gGameTimer.timeSinceLastUpdate = 0.0;

	gInputManager = input_createManager();

	gWorld = world_init();

	// Get started!
	windowDidResize((int)viewport.w, (int)viewport.h);
	while(!_shouldExit) {
		// Feed events to the game thread
		SDL_Event event;
		while (SDL_PollEvent(&event))
			handleEvent(event);

		double delta = 0.0;
		Uint32 currentTime = SDL_GetTicks();
		if(lastTime > 0) {
			Uint32 deltaMsec = currentTime - lastTime;
			delta = (double)deltaMsec / (double)MSEC_PER_SEC;
			gameTimer_update(&gGameTimer, delta);
		}
		lastTime = currentTime;

		// Update the game state as many times as we need to catch up
		for(int i = 0; (i < MAX_FRAMESKIP) && !gameTimer_reachedNextUpdate(&gGameTimer); ++i) {
			input_postActiveEvents(gInputManager);
			world_update(gWorld, gGameTimer.desiredInterval);
			gameTimer_finishedUpdate(&gGameTimer);
		}


		glClear(GL_COLOR_BUFFER_BIT);
		renderer_display(gRenderer, gGameTimer.timeSinceLastUpdate, gameTimer_interpolationSinceLastUpdate(&gGameTimer));
		SDL_GL_SwapBuffers();
	}

	return 0;
}
