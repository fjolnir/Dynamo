#include "json.h"
#include "util.h"
#include <yajl/yajl_parse.h>
#include <yajl/yajl_gen.h>
#include <string.h>
#include <stdlib.h>


#pragma mark - Parsing

struct _ParseContext {
	Obj_t *container;
	char *key; // In case of a map_key, this is set to the key to assign to
};

#define _CTX_SET(ctx, val) \
	if(obj_isClass(ctx->container, &Class_Array)) \
		array_push(ctx->container, val); \
	else if(obj_isClass(ctx->container, &Class_Dictionary)) \
		dict_set(ctx->container, ctx->key, val); \
	else \
		return yajl_status_error;

static int handle_null(void *ctxStack_)
{
	// Do nothing
	return true;
}

static int handle_boolean(void *ctxStack_, int boolean)
{
	struct _ParseContext *ctx = array_top(ctxStack_);
	Number_t *num = number_create(boolean ? 1.0 : 0.0);
	_CTX_SET(ctx, num)
	return true;
}

static int handle_int(void *ctxStack_, long long integerVal)
{
	struct _ParseContext *ctx = array_top(ctxStack_);
	Number_t *num = number_create((double)integerVal);
	_CTX_SET(ctx, num)
	return true;
}

static int handle_double(void *ctxStack_, double doubleVal)
{
	struct _ParseContext *ctx = array_top(ctxStack_);
	Number_t *num = number_create(doubleVal);
	_CTX_SET(ctx, num)
	return true;
}

static int handle_string(void *ctxStack_, const unsigned char *stringVal, size_t stringLen)
{
	struct _ParseContext *ctx = array_top(ctxStack_);
	String_t *str = string_create((char*)stringVal, stringLen);
	_CTX_SET(ctx, str)
	return true;
}

static int handle_map_key(void *ctxStack_, const unsigned char *stringVal, size_t stringLen)
{
	struct _ParseContext *ctx = array_top(ctxStack_);
	ctx->key = calloc(1, stringLen+1);
    strncpy(ctx->key, (char*)stringVal, stringLen);
	return true;
}

static int handle_start_map(void *ctxStack_)
{
	Array_t *ctxStack = ctxStack_;
    
	struct _ParseContext *newCtx = calloc(1, sizeof(struct _ParseContext));
	newCtx->container = dict_create((InsertionCallback_t)&obj_retain, (RemovalCallback_t)&obj_release);
    if(ctxStack->count > 0) {
        struct _ParseContext *parenCtx = array_top(ctxStack);
        _CTX_SET(parenCtx, newCtx->container)
    }
	array_push(ctxStack, newCtx);
	return true;
}

static int handle_start_array(void *ctxStack_)
{
	Array_t *ctxStack = ctxStack_;
	struct _ParseContext *newCtx = calloc(1, sizeof(struct _ParseContext));
	newCtx->container = array_create(4, (InsertionCallback_t)&obj_retain, (RemovalCallback_t)&obj_release);
    if(ctxStack->count > 0) {
        struct _ParseContext *parenCtx = array_top(ctxStack);
        _CTX_SET(parenCtx, newCtx->container)
    }
	array_push(ctxStack, newCtx);

	return true;
}

static int handle_end(void *ctxStack_)
{
    Array_t *ctxStack = ctxStack_;
	// We want to leave the last item on the stack dangling so that we can retrieve the root in parseJSON
	// (Popping the last item means the file has ended)
	if(ctxStack->count > 1)
		array_pop(ctxStack);
	return true;
}
#undef _CTX_SET

static yajl_callbacks parserCallbacks = {
    handle_null,
    handle_boolean,
    handle_int,
    handle_double,
    NULL,
    handle_string,
    handle_start_map,
    handle_map_key,
    handle_end,
    handle_start_array,
    handle_end
};

static void _freeParseContext(struct _ParseContext *ctx)
{
    if(ctx->key)
        free(ctx->key);
    free(ctx);
}

Obj_t *parseJSON(const char *aJsonStr)
{
	Array_t *ctxStack = array_create(8, NULL, (RemovalCallback_t)&_freeParseContext);
	yajl_handle parser = yajl_alloc(&parserCallbacks, NULL, (void *)ctxStack);
	yajl_status status = yajl_parse(parser, (const unsigned char*)aJsonStr, strlen(aJsonStr));
    if(status != yajl_status_ok) {
        unsigned char *err = yajl_get_error(parser, 1, (unsigned char*)aJsonStr, strlen(aJsonStr));
        dynamo_log("Couldn't parse JSON: %s", err);
        yajl_free_error(parser, err);
        return NULL;
    }

	Obj_t *root = NULL;
	if(ctxStack->count == 1) {
        struct _ParseContext *ctx = ctxStack->items[0];
		root = ctx->container;
		array_pop(ctxStack);
	} else {
		dynamo_log("Something went wrong parsing json!");
		return NULL;
	}

	return root;
}

#pragma mark - Generation

static bool _objToJSON(Obj_t *aObj, yajl_gen g);

static void _addDict(char *key, void *value, void *ctx)
{
    yajl_gen_string(ctx, (unsigned char*)key, strlen(key));
    _objToJSON(value, ctx);
}

bool _objToJSON(Obj_t *aObj, yajl_gen g)
{
	if(!aObj) {
		yajl_gen_null(g);
	} else if(obj_isClass(aObj, &Class_Dictionary)) {
		yajl_gen_map_open(g);
		dict_apply(aObj, (DictionaryApplier_t)&_addDict, g);
		yajl_gen_map_close(g);
	} else if(obj_isClass(aObj, &Class_Array)) {
		Array_t *arr = aObj;
		yajl_gen_array_open(g);
		for(int i = 0; i < arr->count; ++i) {
			_objToJSON(arr->items[i], g);
		}
		yajl_gen_array_close(g);
	} else if(obj_isClass(aObj, &Class_String)) {
		String_t *str = aObj;
		yajl_gen_string(g, (unsigned char*)str->cString, str->length);
	} else if(obj_isClass(aObj, &Class_Number)) {
		Number_t *num = aObj;
		yajl_gen_double(g, num->doubleValue);
	} else
		return false;
	return true;
}

bool objToJSON(Obj_t *aObj, char *aoBuf, size_t aBufLen)
{
	yajl_gen g = yajl_gen_alloc(NULL);
//	yajl_gen_config(g, yajl_gen_beautify, 1);
	yajl_gen_config(g, yajl_gen_validate_utf8, 1);

	bool succeeded = _objToJSON(aObj, g);
	if(!succeeded)
		return false;

	const unsigned char *internalBuf;
	size_t internalBufLen;
	yajl_gen_status status = yajl_gen_get_buf(g, &internalBuf, &internalBufLen);
	if(status != yajl_gen_status_ok || aBufLen < internalBufLen+1)
		return false;
	strncpy(aoBuf, (char*)internalBuf, internalBufLen);
    aoBuf[internalBufLen] = '\0';

	return true;
}
