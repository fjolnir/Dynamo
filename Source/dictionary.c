#include "dictionary.h"
#include <string.h>
#include <stdlib.h>
#include <assert.h>

static DictionaryNode_t *_dict_createNode(void *value);
static void dict_destroy(Dictionary_t *aDict);
static DictionaryNode_t *_dict_searchFromNode(DictionaryNode_t *aNode, const char *key, bool createPath);

Class_t Class_Dictionary = {
	"Dictionary",
	sizeof(Dictionary_t),
	(Obj_destructor_t)&dict_destroy
};


Dictionary_t *dict_create(DictionaryInsertionCallback_t aInsertionCallback, DictionaryRemovalCallback_t aRemovalCallback)
{
	Dictionary_t *self = obj_create_autoreleased(&Class_Dictionary);
	self->insertionCallback = aInsertionCallback;
	self->removalCallback = aRemovalCallback;
	self->rootNode.value = NULL;
	memset(self->rootNode.children, 0, DICTNODE_MAXCHILDREN*sizeof(char));
	return self;
}

// Frees all subnodes and removes their values (invoking the dictionary's removal callback if one exists)
static void _dict_cleanNode(Dictionary_t *aDict, DictionaryNode_t *aNode)
{
	DictionaryNode_t *child;
	for(int i = 0; i < DICTNODE_MAXCHILDREN; ++i) {
		child = aNode->children[i];
		if(!child)
			continue;
		_dict_cleanNode(aDict, child);
		free(child);
	}
	if(aDict->removalCallback && aNode->value)
		aDict->removalCallback(aNode->value);
	aNode->value = NULL;
}

static void dict_destroy(Dictionary_t *aDict)
{
	_dict_cleanNode(aDict, &aDict->rootNode);
}

void *dict_get(Dictionary_t *aDict, const char *aKey)
{
	DictionaryNode_t *node = _dict_searchFromNode(&aDict->rootNode, aKey, false);
	if(node)
		return node->value;
	return NULL;
}

void dict_set(Dictionary_t *aDict, const char *aKey, void *aValue)
{
	if(aDict->insertionCallback)
		aDict->insertionCallback(aValue);
	DictionaryNode_t *node = _dict_searchFromNode(&aDict->rootNode, aKey, true);
	node->value = aValue;
}

bool dict_remove(Dictionary_t *aDict, const char *aKey)
{
	DictionaryNode_t *node = _dict_searchFromNode(&aDict->rootNode, aKey, false);
	void *value = node->value;
	node->value = NULL;
	if(aDict->removalCallback)
		aDict->removalCallback(value);
	return value != NULL;
}

static DictionaryNode_t *_dict_searchFromNode(DictionaryNode_t *aNode, const char *aKey, bool aCreatePath)
{
	assert(*aKey >= 0);
	unsigned char key = *aKey;
	if(aNode->children[key] != NULL) {
		if(key == '\0')
			return aNode->children[key];
		else
			return _dict_searchFromNode(aNode->children[key], aKey + sizeof(char), aCreatePath);
	} else if(aCreatePath == true) {
		aNode->children[key] = _dict_createNode(NULL);
		if(key == '\0')
			return aNode->children[key];
		else
			return _dict_searchFromNode(aNode->children[key], aKey + sizeof(char), aCreatePath);
	}
	return NULL;
}

static DictionaryNode_t *_dict_createNode(void *value)
{
	DictionaryNode_t *node = calloc(1, sizeof(DictionaryNode_t));
	node->value = value;
	return node;
}

