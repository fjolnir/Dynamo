/*!
	@header Background
	@abstract
	@discussion A trie based dictionary (defined over ascii: 0-127)
*/

#ifndef _DICTIONARY_H_
#define _DICTIONARY_H_

#include "object.h"
#include "array.h"

#define DICTNODE_MAXCHILDREN (128)
#define DICT_MAXKEYLEN (32)

extern Class_t Class_Dictionary;

typedef struct _Dictionary Dictionary_t;

/*!
	Creates a dictionary.

	@param aInsertionCallback A callback function that is called when an item is inserted into the dictionary.
	@param aRemovalCallback A callback function that is called when an item is inserted into the dictionary.
*/
Dictionary_t *dict_create(InsertionCallback_t aInsertionCallback, RemovalCallback_t aRemovalCallback);
/*!
	Gets the value for a given key.
*/
void *dict_get(Dictionary_t *aDict, const char *aKey);
/*!
	Sets the value for a given key.
*/
void dict_set(Dictionary_t *aDict, const char *aKey, void *aValue);
/*!
	Removes the value for a given key
*/
bool dict_remove(Dictionary_t *aDict, const char *aKey);

/*!
	Dictionary appliers are used to perform an action on each and every item in a dictionary
*/
typedef void (*DictionaryApplier_t)(const char *aKey, void *aValue, void *aCtx);
/*!
	Iterates each value in the dictionary, applying the supplied Applier to it/
*/
void dict_apply(Dictionary_t *aDict, DictionaryApplier_t aApplier, void *aCtx);
#endif
