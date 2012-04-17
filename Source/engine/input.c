#include "input.h"
#include "various.h"

static void input_destroyManager(InputManager_t *aManager);

InputManager_t *input_createManager()
{
	InputManager_t *out = obj_create_autoreleased(sizeof(InputManager_t), (Obj_destructor_t)&input_destroyManager);
	out->observers = obj_retain(llist_create());
	out->activeEvents = obj_retain(llist_create());

	return out;
}

void input_destroyManager(InputManager_t *aManager)
{
	llist_apply(aManager->observers, &obj_release);
	obj_release(aManager->observers);
	aManager->observers = NULL;
	obj_release(aManager->activeEvents);
	aManager->activeEvents = NULL;
}

InputObserver_t *input_createObserver(Input_type_t aObservedType, Input_handler_t aHandlerCallback, char *aCode, void *aMetaData)
{
	InputObserver_t *out = obj_create_autoreleased(sizeof(InputObserver_t), NULL);
	out->type = aObservedType;
	out->handlerCallback = aHandlerCallback;
	out->metaData = aMetaData;
	out->lastKnownState = kInputState_up;
	if(aCode) out->code = *aCode;

	return out;
}

void input_addObserver(InputManager_t *aManager, InputObserver_t *aObserver)
{
	obj_retain(aObserver);
	llist_pushValue(aManager->observers, aObserver);
}
bool input_removeObserver(InputManager_t *aManager, InputObserver_t *aObserver)
{
	obj_release(aObserver);
	return llist_deleteValue(aManager->observers, aObserver);
}

static InputObserver_t **_input_observersForEvent(InputManager_t *aManager, Input_type_t aType, unsigned char *aCode, int *aoCount)
{
	InputObserver_t **out = malloc(sizeof(InputObserver_t *)*MAX_SIMUL_OBSERVERS);
	int count = 0;
	LinkedListItem_t *item = aManager->observers->head;
	if(item) {
		InputObserver_t *observer;
		do {
			observer = (InputObserver_t *)item->value;
			if(observer->type == aType && (!aCode || (observer->code == *aCode)))
				out[count++] = observer;
		} while( (item = item->next) && count < MAX_SIMUL_OBSERVERS);
	}
	if(aoCount) *aoCount = count;
	if(count == 0) {
		free(out);
		return NULL;
	}

	return out;
}
void input_postMomentaryEvent(InputManager_t *aManager, Input_type_t aType, unsigned char *aCode, vec2_t *aLocation, Input_state_t aState)
{
	int count;
	InputObserver_t **observers = _input_observersForEvent(aManager, aType, aCode, &count);
	for(int i = 0; i < count; ++i) {
		observers[i]->handlerCallback(aManager, observers[i], aLocation, aState, observers[i]->metaData);
		observers[i]->lastKnownState = aState;
	}
	free(observers);
}

typedef struct _InputEvent {
	int observerCount;
	InputObserver_t *observers[MAX_SIMUL_OBSERVERS];
	vec2_t location;
	int fireCount;
	Input_state_t state;
	Input_type_t type;
	unsigned char code;
} _InputEvent_t;

void input_postActiveEvents(InputManager_t *aManager)
{
	LinkedListItem_t *item = aManager->activeEvents->head;
	InputObserver_t *obs;
	if(item) {
		_InputEvent_t *event;
		do {
			event = (_InputEvent_t *)item->value;
			for(int i = 0; i < event->observerCount; ++i) {
				obs = event->observers[i];
				obs->handlerCallback(aManager, obs, &event->location, event->state, obs->metaData);
				obs->lastKnownState = event->state;
			}
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
			if(event->type == aType && (!aCode || (event->code == *aCode)) ) {
				if(aoEvent) *aoEvent = event;
				return true;
			}
		} while( (item = item->next) );
	}
	return false;
}
void input_beginEvent(InputManager_t *aManager, Input_type_t aType, unsigned char *aCode, vec2_t *aLocation)
{
	int observerCount;
	InputObserver_t **observers = _input_observersForEvent(aManager, aType, aCode, &observerCount);
	_InputEvent_t *existingEvent;
	bool isActive = _input_eventIsActive(aManager, aType, aCode, &existingEvent);
	// Keep the location up to date if the event is already active
	if(isActive && aLocation) existingEvent->location = *aLocation;

	if(!observers || isActive)
		return;

	_InputEvent_t *event = malloc(sizeof(_InputEvent_t));
	event->observerCount = observerCount;
	event->type = aType;
	if(aCode) event->code = *aCode;
	for(int i = 0; i < observerCount; ++i) {
		event->observers[i] = observers[i];
	}
	free(observers);
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
	Input_type_t state = event->state == kInputState_down ? kInputState_up : kInputState_down;
	input_postMomentaryEvent(aManager, aType, aCode, &event->location, state);
	llist_deleteValue(aManager->activeEvents, event);
	free(event);
}
