// A trie based dictionary (defined over ascii: 0-127)

#ifndef _DICTIONARY_H_
#define _DICTIONARY_H_

#include "object.h"
#include "array.h"

#define DICTNODE_MAXCHILDREN (128)

extern Class_t Class_Dictionary;

typedef struct _Dictionary Dictionary_t;
typedef struct _DictionaryNode DictionaryNode_t;

// Called when values are inserted&removed. Usage example: aDict = dict_create(&obj_retain, &obj_release);
typedef void (*DictionaryInsertionCallback_t)(void *aVal);
typedef void (*DictionaryRemovalCallback_t)(void *aVal);

struct _DictionaryNode {
	void *value;
	DictionaryNode_t *children[DICTNODE_MAXCHILDREN];
};

struct _Dictionary {
	OBJ_GUTS
	DictionaryNode_t rootNode;
	DictionaryInsertionCallback_t insertionCallback;
	DictionaryRemovalCallback_t removalCallback;
};

Dictionary_t *dict_create(DictionaryInsertionCallback_t aInsertionCallback, DictionaryRemovalCallback_t aRemovalCallback);
void *dict_get(Dictionary_t *aDict, const char *aKey);
void dict_set(Dictionary_t *aDict, const char *aKey, void *aValue);
bool dict_remove(Dictionary_t *aDict, const char *aKey);
#endif
