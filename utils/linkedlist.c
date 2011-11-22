#include "linkedlist.h"
#include <stdlib.h>


#pragma mark -

LinkedList_t *llist_create()
{
	return calloc(1, sizeof(LinkedList_t));
}

void llist_destroy(LinkedList_t *aList, bool aShouldFreeValues)
{
	LinkedListItem_t *currentItem = aList->head;
	if(currentItem) {
		LinkedListItem_t *temp;
		do {
			if(aShouldFreeValues) free(currentItem->value);

			temp = currentItem;
			currentItem = temp->next;
			free(temp);
		} while(currentItem);
	}
	free(aList);
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

void llist_popValue(LinkedList_t *aList)
{
	LinkedListItem_t *tail = aList->tail;
	if(!tail) return;

	tail->previous->next = NULL;
	aList->tail = tail->previous;
	free(tail);
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
