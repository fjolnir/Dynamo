// Note: linked lists do not call obj_retain on their values (since they support arbitrary pointers)
#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

typedef struct _LinkedList LinkedList_t;
typedef struct _LinkedListItem LinkedListItem_t;
#include "object.h"
#include <stdbool.h>



struct _LinkedList {
	OBJ_GUTS
	LinkedListItem_t *head;
	LinkedListItem_t *tail;
};

struct _LinkedListItem {
	LinkedListItem_t *previous, *next;
	void *value;
};

extern LinkedList_t *llist_create();

extern void llist_pushValue(LinkedList_t *aList, void *aValue);
extern void *llist_popValue(LinkedList_t *aList);
extern bool llist_insertValue(LinkedList_t *aList, void *aValueToInsert, void *aValueToShift);
extern bool llist_deleteValue(LinkedList_t *aList, void *aValue);
void llist_empty(LinkedList_t *aList);

typedef void (*LinkedListApplier_t)(void *aValue);
extern void llist_apply(LinkedList_t *aList, LinkedListApplier_t aApplier); 
#endif

