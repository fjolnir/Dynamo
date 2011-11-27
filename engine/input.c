#include "input.h"

InputManager_t *input_createManager()
{
	InputManager_t *out = malloc(sizeof(InputManager_t));
	out->observers = llist_create();
	out->activeEvents = llist_create();

	return out;
}

void input_destroyManager(InputManager_t *aManager)
{
	llist_destroy(aManager->observers, true);
	free(aManager);
}

InputObserver_t *input_createObserver(Input_type_t aObservedType, Input_handler_t aHandlerCallback, char *aCode, void *aMetaData)
{
	InputObserver_t *out = malloc(sizeof(InputObserver_t));
	out->type = aObservedType;
	out->handlerCallback = aHandlerCallback;
	out->metaData = aMetaData;
	if(aCode) out->code = *aCode;

	return out;
}
void input_destroyObserver(InputObserver_t *aObserver)
{
	free(aObserver);
}

void input_addObserver(InputManager_t *aManager, InputObserver_t *aObserver)
{
	llist_pushValue(aManager->observers, aObserver);
}
bool input_removeObserver(InputManager_t *aManager, InputObserver_t *aObserver)
{
	return llist_deleteValue(aManager->observers, aObserver);
}

static InputObserver_t *_input_observerForEvent(InputManager_t *aManager, Input_type_t aType, unsigned char *aCode)
{
	LinkedListItem_t *item = aManager->observers->head;
	if(item) {
		InputObserver_t *observer;
		do {
			observer = (InputObserver_t *)item->value;
			if(observer->type == aType && (!aCode || (observer->code == *aCode)))
				return observer;
		} while( (item = item->next) );
	}
	return NULL;
}
void input_postMomentaryEvent(InputManager_t *aManager, Input_type_t aType, unsigned char *aCode, vec2_t *aLocation, Input_state_t aState)
{
	InputObserver_t *observer = _input_observerForEvent(aManager, aType, aCode);
	if(observer) observer->handlerCallback(aManager, observer, aLocation, aState, observer->metaData);
}

typedef struct _InputEvent {
	InputObserver_t *observer;
	vec2_t location;
	int fireCount;
	Input_state_t state;
} _InputEvent_t;

void input_postActiveEvents(InputManager_t *aManager)
{
	LinkedListItem_t *item = aManager->activeEvents->head;
	if(item) {
		_InputEvent_t *event;
		do {
			event = (_InputEvent_t *)item->value;
			event->observer->handlerCallback(aManager, event->observer, &event->location, event->state, event->observer->metaData);
			event->fireCount++;
		} while( (item = item->next) );
	}
}
static bool _input_eventIsActive(InputManager_t *aManager, Input_type_t aType, unsigned char *aCode, _InputEvent_t **aoEvent)
{
	LinkedListItem_t *item = aManager->activeEvents->head;
	if(item) {
		_InputEvent_t *event;
		do {
			event = (_InputEvent_t *)item->value;
			if(event->observer->type == aType && (!aCode || (event->observer->code == *aCode)) ) {
				if(aoEvent) *aoEvent = event;
				return true;
			}
		} while( (item = item->next) );
	}
	return false;
}
void input_beginEvent(InputManager_t *aManager, Input_type_t aType, unsigned char *aCode, vec2_t *aLocation)
{
	InputObserver_t *observer = _input_observerForEvent(aManager, aType, aCode);
	_InputEvent_t *existingEvent;
	bool isActive = _input_eventIsActive(aManager, aType, aCode, &existingEvent);
	// Keep the location up to date if the event is already active
	if(isActive && aLocation) existingEvent->location = *aLocation;
	
	if(!observer || isActive)
		return;
	_InputEvent_t *event = malloc(sizeof(_InputEvent_t));
	event->observer = observer;
	if(aLocation != NULL) event->location = *aLocation;
	event->fireCount = 0;
	event->state = kInputState_down;

	llist_pushValue(aManager->activeEvents, event);
}
void input_endEvent(InputManager_t *aManager, Input_type_t aType, unsigned char *aCode)
{
	_InputEvent_t *event;
	if(!_input_eventIsActive(aManager, aType, aCode, &event))
		return;
	input_postMomentaryEvent(aManager, aType, aCode, &event->location, event->state);
	llist_deleteValue(aManager->activeEvents, event);
	free(event);
}
