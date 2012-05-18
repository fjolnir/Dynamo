// Note: linked lists do not call obj_retain on their values (since they support arbitrary pointers)
#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

typedef struct _LinkedList LinkedList_t;
typedef struct _LinkedListItem LinkedListItem_t;

#include "object.h"
#include "util.h"
#include <stdbool.h>

struct _LinkedList {
	OBJ_GUTS
	LinkedListItem_t *head;
	LinkedListItem_t *tail;
	InsertionCallback_t insertionCallback;
	RemovalCallback_t removalCallback;
};

extern Class_t Class_LinkedList;
struct _LinkedListItem {
	LinkedListItem_t *previous, *next;
	void *value;
};

/*!
	Creates a linked list.

	@param aInsertionCallback A callback function that is called when an item is inserted into the list.
	@param aRemovalCallback A callback function that is called when an item is inserted into the list.
*/
extern LinkedList_t *llist_create(InsertionCallback_t aInsertionCallback, RemovalCallback_t aRemovalCallback);

/*!
	Pushes a value onto the end of the list.
*/
extern void llist_pushValue(LinkedList_t *aList, void *aValue);
/*!
	Pops a value off the end of the list.
*/
extern void *llist_popValue(LinkedList_t *aList);
/*!
	Inserts a value before a different value in the list
	@param aValueToInsert The value to insert
	@param aValueToShift The value to be shifted to the right by the inserted value.
*/
extern bool llist_insertValue(LinkedList_t *aList, void *aValueToInsert, void *aValueToShift);
/*!
	Deletes a value from the list.
*/
extern bool llist_deleteValue(LinkedList_t *aList, void *aValue);
/*!
	Deletes all values from the list.
*/
void llist_empty(LinkedList_t *aList);

/*!
	Appliers are used to perform an action on each and every item in a list
*/
typedef void (*LinkedListApplier_t)(void *aValue, void *aCtx);
/*!
	Iterates each value in the list, applying the supplied Applier to it.
*/
extern void llist_apply(LinkedList_t *aList, LinkedListApplier_t aApplier, void *aCtx);
#endif

