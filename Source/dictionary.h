// A trie based dictionary (defined over ascii: 0-127)

#ifndef _DICTIONARY_H_
#define _DICTIONARY_H_

#include "object.h"
#include "array.h"

#define DICTNODE_MAXCHILDREN (128)
#define DICT_MAXKEYLEN (32)

extern Class_t Class_Dictionary;

typedef struct _Dictionary Dictionary_t;
typedef struct _DictionaryNode DictionaryNode_t;

struct _DictionaryNode {
	void *value;
	DictionaryNode_t *children[DICTNODE_MAXCHILDREN];
};

struct _Dictionary {
	OBJ_GUTS
	DictionaryNode_t rootNode;
	InsertionCallback_t insertionCallback;
	RemovalCallback_t removalCallback;
};

Dictionary_t *dict_create(InsertionCallback_t aInsertionCallback, RemovalCallback_t aRemovalCallback);
void *dict_get(Dictionary_t *aDict, const char *aKey);
void dict_set(Dictionary_t *aDict, const char *aKey, void *aValue);
bool dict_remove(Dictionary_t *aDict, const char *aKey);

typedef void (*DictionaryApplier_t)(const char *aKey, void *aValue, void *aCtx);
void dict_apply(Dictionary_t *aDict, DictionaryApplier_t aApplier, void *aCtx);
#endif
