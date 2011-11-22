// A mechanism to define callbacks for keyboard events
// Does not actually provide a method for listening to the keyboard.
// Rather you would pipe events from whatever middleware framework or library you are using
// on your platform

#include "linkedlist.h"
#include "GLMath/GLMath.h"

#ifndef _INPUT_H_
#define _INPUT_H_

typedef enum {
	kInputKey_arrowLeft,
	kInputKey_arrowRight,
	kInputKey_arrowUp,
	kInputKey_arrowDown,
	kInputKey_ascii,
	kInputMouse_leftClick,
	kInputMouse_rightClick,
	kInputMouse_leftDrag,
	kInputMouse_rightDrag,
	kInputMouse_move
} Input_type_t;

typedef enum {
	kInputState_down,
	kInputState_up
} Input_state_t;

typedef struct _InputManager InputManager_t;
typedef struct _InputObserver InputObserver_t;

typedef void (*Input_handler_t)(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *aMetaData);

struct _InputObserver {
	Input_handler_t handlerCallback;
	Input_type_t type;
	unsigned char code; // Used for ASCII keys
	void *metaData; // Arbitrary pointer for providing context
};
struct _InputManager {
	LinkedList_t *observers;
	LinkedList_t *activeEvents;
};

extern InputManager_t *input_createManager();
extern void input_destroyManager(InputManager_t *aManager);
extern InputObserver_t *input_createObserver(Input_type_t aObservedType, Input_handler_t aHandlerCallback, char *aCode, void *aMetaData);
extern void input_destroyObserver(InputObserver_t *aObserver);

extern void input_addObserver(InputManager_t *aManager, InputObserver_t *aObserver);
extern bool input_removeObserver(InputManager_t *aManager, InputObserver_t *aObserver);


// Called from within the run loop to post an instance of each active event (held keys)
extern void input_postActiveEvents(InputManager_t *aManager);
// Calls any observers for a certain type of input
extern void input_postMomentaryEvent(InputManager_t *aManager, Input_type_t aType, unsigned char *aCode, vec2_t *aLocation, Input_state_t aState);
// Activates an event so it gets posted once per cycle
extern void input_beginEvent(InputManager_t *aManager, Input_type_t aType, unsigned char *aCode, vec2_t *aLocation);
extern void input_endEvent(InputManager_t *aManager, Input_type_t aType, unsigned char *aCode);
#endif
