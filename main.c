// Just handles the runloop

#include <stdio.h>
#include "engine/glutils.h"
#include "shared.h"

#ifdef WIN32
	#define _CRT_SECURE_NO_DEPRECATE
	#define _WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include "engine/windows/gldefs.h"
#endif

// Globals
Renderer_t *gRenderer;
World_t *gWorld;
GameTimer_t gGameTimer;
InputManager_t *gInputManager;
SoundManager_t *gSoundManager;

// Locals
static bool _leftMouseButtonDown, _rightMouseButtonDown;

#pragma mark - Drawing

static void display()
{
	gameTimer_update(&gGameTimer, timeInUsec()/1000);

	// Update the game state as many times as we need to catch up
	while(gGameTimer.timeSinceLastUpdate >= 0){//gGameTimer.desiredInterval) { Feels better like this, eliminates occasional chunkiness (Needs more testing)
		input_postActiveEvents(gInputManager);
		world_update(gWorld);
		gGameTimer.timeSinceLastUpdate -= gGameTimer.desiredInterval;
	}

	renderer_display(gRenderer);

	glutSwapBuffers();
	glFlush();
	glutPostRedisplay();
}

static void windowDidResize(int aWidth, int aHeight)
{
	gRenderer->viewportSize.w = (float)aWidth;
	gRenderer->viewportSize.h = (float)aHeight;

	matrix_stack_pop(gRenderer->projectionMatrixStack);
	matrix_stack_push_item(gRenderer->projectionMatrixStack, mat4_ortho(0.0f, (float)aWidth, 0.0f, (float)aHeight, -1.0f, 1.0f));
}


#pragma mark - Event handling

static void keyWasPressed(unsigned char aKey, int aMouseX, int aMouseY)
{
	vec2_t location = { (float)aMouseX, gRenderer->viewportSize.h - (float)aMouseY };
	input_beginEvent(gInputManager, kInputKey_ascii, &aKey, &location);
}
static void keyWasReleased(unsigned char aKey, int aMouseX, int aMouseY)
{
	input_endEvent(gInputManager, kInputKey_ascii, &aKey);
}

static void specialKeyWasPressed(int aKey, int aMouseX, int aMouseY)
{
	Input_type_t inputType;
	switch(aKey) {
		case GLUT_KEY_LEFT:
			inputType = kInputKey_arrowLeft;
			break;
		case GLUT_KEY_RIGHT:
			inputType = kInputKey_arrowRight;
			break;
		case GLUT_KEY_UP:
			inputType = kInputKey_arrowUp;
			break;
		case GLUT_KEY_DOWN:
			inputType = kInputKey_arrowDown;
			break;
		default:
			return;
	}
	vec2_t location = { (float)aMouseX, (float)aMouseY };
	input_beginEvent(gInputManager, inputType, NULL, &location);
}
static void specialKeyWasReleased(int aKey, int aMouseX, int aMouseY)
{
	Input_type_t inputType;
	switch(aKey) {
		case GLUT_KEY_LEFT:
			inputType = kInputKey_arrowLeft;
			break;
		case GLUT_KEY_RIGHT:
			inputType = kInputKey_arrowRight;
			break;
		case GLUT_KEY_UP:
			inputType = kInputKey_arrowUp;
			break;
		case GLUT_KEY_DOWN:
			inputType = kInputKey_arrowDown;
			break;
		default:
			return;
	}
	input_endEvent(gInputManager, inputType, NULL);
}

static void mouseMoved(int aMouseX, int aMouseY)
{
	vec2_t location = { (float)aMouseX, gRenderer->viewportSize.h - (float)aMouseY };
	input_postMomentaryEvent(gInputManager, kInputMouse_move, NULL, &location, kInputState_up);
}
static void mouseDragged(int aMouseX, int aMouseY)
{
	vec2_t location = { (float)aMouseX, gRenderer->viewportSize.h - (float)aMouseY };
	if(_leftMouseButtonDown)
		input_postMomentaryEvent(gInputManager, kInputMouse_leftDrag, NULL, &location, kInputState_down);
	if(_rightMouseButtonDown)
		input_postMomentaryEvent(gInputManager, kInputMouse_rightDrag, NULL, &location, kInputState_down);
}

static void mouseButtonClicked(int aButton, int aState, int aMouseX, int aMouseY)
{
	vec2_t location = { (float)aMouseX, gRenderer->viewportSize.h - (float)aMouseY };
	Input_type_t inputType;
	switch(aButton) {
		case GLUT_LEFT_BUTTON:
			_leftMouseButtonDown = (aState == GLUT_DOWN);
			inputType = kInputMouse_leftClick;
			break;
		case GLUT_RIGHT_BUTTON:
			_rightMouseButtonDown = (aState == GLUT_DOWN);
			inputType = kInputMouse_rightClick;
			break;
		default:
			return;
	}
	input_postMomentaryEvent(gInputManager, inputType, NULL,
	                         &location, (aState == GLUT_DOWN) ? kInputState_down : kInputState_up);
}


#pragma mark - Initialization

static void cleanup()
{
	soundManager_destroy(gSoundManager);
	world_destroy(gWorld);
	renderer_destroy(gRenderer);
	input_destroyManager(gInputManager);
	draw_cleanup();
	exit(0);
}

int main(int argc, char **argv)
{
	gSoundManager = soundManager_create();
//	Sound_t *testSound = sound_load("audio/test.ogg");
//	sound_play(testSound);

	vec2_t viewport = { 640.0f, 480.0f };

	// Initialize graphics
	glutInit(&argc, argv);
	glutInitWindowPosition(64,64);
	glutInitWindowSize(viewport.w, viewport.h);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA);
	glutCreateWindow("");

	gRenderer = renderer_create(viewport, kVec3_zero);
	draw_init(gRenderer);

	gGameTimer.desiredInterval = 1000.0/(double)DESIRED_FPS;
	gGameTimer.elapsed = timeInUsec();
	gGameTimer.estimatedFPS = 0.0;

	gInputManager = input_createManager();

	gWorld = world_init();

#ifdef GL_GLEXT_PROTOTYPES
	loadGLExtensions(); // Load GL on windows
#endif

	glutReshapeFunc(windowDidResize);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyWasPressed);
	glutKeyboardUpFunc(keyWasReleased);
	glutSpecialFunc(specialKeyWasPressed);
	glutSpecialUpFunc(specialKeyWasReleased);
	glutMouseFunc(mouseButtonClicked);
	glutMotionFunc(mouseDragged);
	glutPassiveMotionFunc(mouseMoved);

	// GL setup
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);

#ifndef WIN32 // TODO: Write equivalent for windows (or cross platform)
	// Make sure we clean up after ourselves before exiting
	signal(SIGINT, cleanup); // While debugging
	//atexit(cleanup);
#endif

	// Enter the runloop
	glutMainLoop();

	return 0;
}
