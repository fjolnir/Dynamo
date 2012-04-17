#include "linkedlist.h"
#include <stdlib.h>

static void llist_destroy(LinkedList_t *aList);

#pragma mark -

LinkedList_t *llist_create()
{
	return obj_create_autoreleased(sizeof(LinkedList_t), (Obj_destructor_t)&llist_destroy);
}

void llist_destroy(LinkedList_t *aList)
{
	LinkedListItem_t *currentItem = aList->head;
	if(currentItem) {
		LinkedListItem_t *temp;
		do {
			temp = currentItem;
			currentItem = temp->next;
			free(temp);
		} while(currentItem);
	}
}


#pragma mark - Data management

static LinkedListItem_t *_llist_itemForValue(LinkedList_t *aList, void *aValue)
{
	LinkedListItem_t *currentItem = aList->head;
	do {
		if(currentItem->value == aValue)
			return currentItem;
	} while( (currentItem = currentItem->next) );

	return NULL;
}

void llist_pushValue(LinkedList_t *aList, void *aValue)
{
	LinkedListItem_t *item = calloc(1, sizeof(LinkedListItem_t));
	item->value = aValue;
	item->previous = aList->tail;
	if(aList->tail) aList->tail->next = item;

	aList->tail = item;
	if(!item->previous) aList->head = item;
}

void *llist_popValue(LinkedList_t *aList)
{
	LinkedListItem_t *tail = aList->tail;
	if(!tail) return NULL;
	void *val = tail->value;
	if(tail->previous)
		tail->previous->next = NULL;
	aList->tail = tail->previous;

	if(aList->head == tail)
		aList->head = NULL;
	free(tail);

	return val;
}

bool llist_insertValue(LinkedList_t *aList, void *aValueToInsert, void *aValueToShift)
{
	LinkedListItem_t *itemToInsert = calloc(1, sizeof(LinkedListItem_t));
	itemToInsert->value = aValueToInsert;
	LinkedListItem_t *itemToShift = _llist_itemForValue(aList, aValueToShift);
	if(itemToShift) {
		itemToInsert->next = itemToShift;
		itemToInsert->previous = itemToShift->previous;
		itemToShift->previous = itemToInsert;
		if(aList->head == itemToShift) aList->head = itemToInsert;

		return true;
	} else {
		free(itemToInsert);
		llist_pushValue(aList, aValueToInsert);

		return false;
	}
}

bool llist_deleteValue(LinkedList_t *aList, void *aValue)
{
	LinkedListItem_t *itemToDelete = _llist_itemForValue(aList, aValue);
	if(itemToDelete) {
		if(itemToDelete->previous) itemToDelete->previous->next = itemToDelete->next;
		if(itemToDelete->next)     itemToDelete->next->previous = itemToDelete->previous;
		if(aList->head == itemToDelete)
			aList->head = itemToDelete->next;
		if(aList->tail == itemToDelete)
			aList->tail = itemToDelete->previous;
		free(itemToDelete);

		return true;
	}
	return false;
}

void llist_empty(LinkedList_t *aList)
{
	while(llist_popValue(aList));
}

void llist_apply(LinkedList_t *aList, LinkedListApplier_t aApplier)
{
	LinkedListItem_t *item = aList->head;
	if(item) {
		do {
			aApplier(item->value);
		} while( (item = item->next));
	}
}
