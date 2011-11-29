#include <stdbool.h>

#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

typedef struct _LinkedListItem LinkedListItem_t;

typedef struct _LinkedList {
	LinkedListItem_t *head;
	LinkedListItem_t *tail;
} LinkedList_t;

struct _LinkedListItem {
	LinkedListItem_t *previous, *next;
	void *value;
};

extern LinkedList_t *llist_create();
extern void llist_destroy(LinkedList_t *aList, bool aShouldFreeValues);

extern void llist_pushValue(LinkedList_t *aList, void *aValue);
extern void llist_popValue(LinkedList_t *aList);
extern bool llist_insertValue(LinkedList_t *aList, void *aValueToInsert, void *aValueToShift);
extern bool llist_deleteValue(LinkedList_t *aList, void *aValue);

#endif

