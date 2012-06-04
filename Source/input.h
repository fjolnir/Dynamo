/*!
	@header Input
	@abstract
	@discussion A mechanism to define callbacks for keyboard events
	Does not actually provide a method for listening to the keyboard.
	Rather you would pipe events from whatever middleware framework or library you are using
	on your platform
*/

// 
#include "object.h"
#include "linkedlist.h"
#include "GLMath/GLMath.h"

#ifndef _INPUT_H_
#define _INPUT_H_

extern Class_t Class_InputManager;
extern Class_t Class_InputObserver;

// Maximum number of observers for a single event
#define MAX_SIMUL_OBSERVERS 8

/*!
	Indicates the type of an event.
*/
typedef enum {
	kInputKey_arrowLeft = 0,
	kInputKey_arrowRight,
	kInputKey_arrowUp,
	kInputKey_arrowDown,
	kInputKey_ascii,
	kInputMouse_leftClick,
	kInputMouse_rightClick,
	kInputMouse_leftDrag,
	kInputMouse_rightDrag,
	kInputMouse_move,

	// Touch events
	kInputTouch1,
	kInputTouch2,
	kInputTouch3,
	kInputTouch4,
	kInputTouch5
} Input_type_t;

/*!
	Indicates the state of an event.
*/
typedef enum {
	kInputState_up,
	kInputState_down
} Input_state_t;

typedef struct _InputManager InputManager_t;
typedef struct _InputObserver InputObserver_t;

typedef void (*Input_handler_t)(InputManager_t *aInputManager, InputObserver_t *aInputObserver, vec2_t *aLocation, Input_state_t aState, void *aMetaData);

/*!
	Observes the event stream for a given event and executes it's handler callback when it encounters it.

	@field handlerCallback The function to be called when the event occurs
	@field type The type of event the observer is interested in
	@field code The character code the observer is interested in (In case of keyboard events, ignored otherwise)
	@field metaData An arbitrary pointer for providing context.
*/
struct _InputObserver {
	OBJ_GUTS
	Input_handler_t handlerCallback;
	Input_type_t type;
	unsigned char code;
	void *metaData;

	Input_state_t lastKnownState;
};

struct _InputManager {
	OBJ_GUTS
	LinkedList_t *observers;
	LinkedList_t *activeEvents;
};

/*!
	Creates an input manager
*/
extern InputManager_t *input_createManager();

/*!
	Creates an input observer (See docs for struct _InputObserver for description of the parameters)
*/
extern InputObserver_t *input_createObserver(Input_type_t aObservedType, Input_handler_t aHandlerCallback, char *aCode, void *aMetaData);
/*!
	Adds an observer to an input manager
*/
extern void input_addObserver(InputManager_t *aManager, InputObserver_t *aObserver);
/*!
	Removes an observer from an input manager
*/
extern bool input_removeObserver(InputManager_t *aManager, InputObserver_t *aObserver);


/*!
	Called from within the run loop to post an instance of each active event (held keys)
*/
extern void input_postActiveEvents(InputManager_t *aManager);
/*!
	Posts a momentary event (That is, it's observers will only see it once)
*/
extern void input_postMomentaryEvent(InputManager_t *aManager, Input_type_t aType, unsigned char *aCode, vec2_t *aLocation, Input_state_t aState);
/*!
	Starts an event (That is, it's observers will see it on every iteration until the event is ended)
*/
extern void input_beginEvent(InputManager_t *aManager, Input_type_t aType, unsigned char *aCode, vec2_t *aLocation);
/*!
	Ends an event
*/
extern void input_endEvent(InputManager_t *aManager, Input_type_t aType, unsigned char *aCode);
#endif
