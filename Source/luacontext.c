#include "luacontext.h"
#include "util.h"
#include <string.h>
#include <stdlib.h>

static void luaCtx_destroy(LuaContext_t *aCtx);

LuaContext_t *GlobalLuaContext = NULL;

int luaApi_dynamo_registerCallback(lua_State *aState);
int luaApi_dynamo_unregisterCallback(lua_State *aState);


Class_t Class_LuaContext = {
    "LuaContext",
    sizeof(LuaContext_t),
    (Obj_destructor_t)&luaCtx_destroy
};

void luaCtx_init()
{
    luaCtx_teardown();
    luaCtx_createContext();
}

void luaCtx_teardown()
{
    if(GlobalLuaContext) {
        obj_release(GlobalLuaContext);
        GlobalLuaContext = NULL;
    }
}

LuaContext_t *luaCtx_createContext()
{
    LuaContext_t *out = obj_create_autoreleased(&Class_LuaContext);
    
    if(!GlobalLuaContext)
        GlobalLuaContext = obj_retain(out);
    
    out->luaState = lua_open();
    luaL_openlibs(out->luaState);
        
    char buf[1024];
    dynamo_assert(util_pathForResource(NULL, NULL, "DynamoScripts", buf, 1024), "Couldn't find dynamo scripts");
    luaCtx_addSearchPath(out, buf);
    
    dynamo_assert(util_pathForResource("glmath", "lua", "DynamoScripts", buf, 1024), "Couldn't find glmath init script");
	dynamo_assert(luaCtx_executeFile(out, buf), "Error initializing GLMath");
    dynamo_assert(util_pathForResource("dynamo", "lua", "DynamoScripts", buf, 1024), "Couldn't find dynamo init script");
	dynamo_assert(luaCtx_executeFile(out, buf), "Error initializing dynamo");
    
    lua_pushcfunction(out->luaState, &luaApi_dynamo_registerCallback);
    lua_setglobal(out->luaState, "dynamo_registerCallback");
    lua_pushcfunction(out->luaState, &luaApi_dynamo_unregisterCallback);
    lua_setglobal(out->luaState, "dynamo_unregisterCallback");
    
    return out;
}

static void luaCtx_destroy(LuaContext_t *aCtx)
{
    lua_close(aCtx->luaState);
}

void luaCtx_getglobal(LuaContext_t *aCtx, const char *k)
{
    lua_getglobal(aCtx->luaState, k);
}

void luaCtx_pop(LuaContext_t *aCtx, int n)
{
    lua_pop(aCtx->luaState, n);
}

void luaCtx_createtable(LuaContext_t *aCtx, int narr, int nrec)
{
    lua_createtable(aCtx->luaState, narr, nrec);
}

void luaCtx_getfield(LuaContext_t *aCtx, int idx, const char *k)
{
    lua_getfield(aCtx->luaState, idx, k);
}

void luaCtx_setfield(LuaContext_t *aCtx, int idx, const char *k)
{
    lua_setfield(aCtx->luaState, idx, k);
}

bool luaCtx_pcall(LuaContext_t *aCtx, int nargs, int nresults, int errfunc)
{
    int err = lua_pcall(aCtx->luaState, nargs, nresults, errfunc);
    if(err) {
        dynamo_log("Lua error: %s", (char*)lua_tostring(aCtx->luaState, -1));
        lua_pop(aCtx->luaState, 1);
        return false;
    }
    return true;
}

bool luaCtx_executeFile(LuaContext_t *aCtx, const char *aPath)
{
    int err = 0;
    err = luaL_loadfile(aCtx->luaState, aPath);
    
    if(err) {
        dynamo_log("Lua error: %s", (char*)lua_tostring(aCtx->luaState, -1));
        lua_pop(aCtx->luaState, 1);
        return false;
    }
    return luaCtx_pcall(aCtx, 0, 0, 0);
}

bool luaCtx_executeString(LuaContext_t *aCtx, const char *aScript)
{
    int err = 0;
    err = luaL_loadstring(aCtx->luaState, aScript);
    if(err) {
        dynamo_log("Lua error: %s", (char*)lua_tostring(aCtx->luaState, -1));
        lua_pop(aCtx->luaState, 1);
        return false;
    }
    return luaCtx_pcall(aCtx, 0, 0, 0);
}

bool luaCtx_addSearchPath(LuaContext_t *aCtx, const char *aPath)
{    
    lua_State *ls = aCtx->luaState;
    lua_getglobal(ls, "package");
        lua_getfield(ls, -1, "path");
        lua_pushfstring(ls, ";%s/?.lua;%s/?/init.lua", aPath, aPath);
        lua_concat(ls, 2);
        lua_setfield(ls, -2, "path");
    lua_pop(ls, 1);
    
    return true;
}

int luaState_registerScriptHandler(lua_State *aLuaState, int aIdx)
{
    if(!lua_isfunction(aLuaState, aIdx)) {
        lua_pushstring(aLuaState, "Handler must be a function");
        lua_error(aLuaState);
        return 1;
    }
    lua_pushvalue(aLuaState, aIdx);
    return luaL_ref(aLuaState, LUA_REGISTRYINDEX);
}

void luaCtx_pushvalue(LuaContext_t *aCtx, int idx)
{
    lua_pushvalue(aCtx->luaState, idx);
}

void luaCtx_pushnil(LuaContext_t *aCtx)
{
    lua_pushnil(aCtx->luaState);
}
void luaCtx_pushnumber(LuaContext_t *aCtx, float n)
{
    lua_pushnumber(aCtx->luaState, n);
}
void luaCtx_pushinteger(LuaContext_t *aCtx, int n)
{
    lua_pushinteger(aCtx->luaState, n);
}
void luaCtx_pushstring(LuaContext_t *aCtx, const char *s)
{
    lua_pushstring(aCtx->luaState, s);
}
void luaCtx_pushboolean(LuaContext_t *aCtx, int b)
{
    lua_pushboolean(aCtx->luaState, b);
}

void luaCtx_pushlightuserdata(LuaContext_t *aCtx, void *ptr)
{
    lua_pushlightuserdata(aCtx->luaState, ptr);
}

bool luaCtx_pushScriptHandler(LuaContext_t *aCtx, int aRefId)
{
    lua_rawgeti(aCtx->luaState, LUA_REGISTRYINDEX, aRefId);
    if(!lua_isfunction(aCtx->luaState, -1)) {
        lua_pop(aCtx->luaState, 1);
        return false;
    }
    return true;
}

void luaCtx_unregisterScriptHandler(LuaContext_t *aCtx, int aRefId)
{
    luaL_unref(aCtx->luaState, LUA_REGISTRYINDEX, aRefId);
}

void luaCtx_concat(LuaContext_t *aCtx, int n)
{
    lua_concat(aCtx->luaState, n);
}
int luaCtx_next(LuaContext_t *aCtx, int index)
{
	return lua_next(aCtx->luaState, index);
}

int luaCtx_type(LuaContext_t *aCtx, int idx)
{
	return lua_type(aCtx->luaState, idx);
}
int luaCtx_isnumber(LuaContext_t *aCtx, int idx)
{
	return lua_isnumber(aCtx->luaState, idx);
}
int luaCtx_isstring(LuaContext_t *aCtx, int idx)
{
	return lua_isstring(aCtx->luaState, idx);
}
int luaCtx_isuserdata(LuaContext_t *aCtx, int idx)
{
	return lua_isuserdata(aCtx->luaState, idx);
}
int luaCtx_istable(LuaContext_t *aCtx, int idx)
{
	return lua_istable(aCtx->luaState, idx);
}
int luaCtx_islightuserdata(LuaContext_t *aCtx, int idx)
{
	return lua_islightuserdata(aCtx->luaState, idx);
}
int luaCtx_isnil(LuaContext_t *aCtx, int idx)
{
	return lua_isnil(aCtx->luaState, idx);
}
int luaCtx_isboolean(LuaContext_t *aCtx, int idx)
{
	return lua_isboolean(aCtx->luaState, idx);
}
int luaCtx_isfunction(LuaContext_t *aCtx, int idx)
{
	return lua_isfunction(aCtx->luaState, idx);
}
int luaCtx_isnone(LuaContext_t *aCtx, int idx)
{
	return lua_isnone(aCtx->luaState, idx);
}


float luaCtx_tonumber(LuaContext_t *aCtx, int idx)
{
	return lua_tonumber(aCtx->luaState, idx);
}
int luaCtx_tointeger(LuaContext_t *aCtx, int idx)
{
	return lua_tointeger(aCtx->luaState, idx);
}
int luaCtx_toboolean(LuaContext_t *aCtx, int idx)
{
	return lua_toboolean(aCtx->luaState, idx);
}
const char *luaCtx_tostring(LuaContext_t *aCtx, int idx)
{
	return lua_tostring(aCtx->luaState, idx);
}


#pragma mark - Lua callback utils
// These are necessary because iOS does not allow an executable stack which breaks luajit ffi callbacks.

int luaApi_dynamo_registerCallback(lua_State *aState)
{
    if(!lua_isfunction(aState, -1)) {
        lua_pushstring(aState, "Callback must be a function");
        lua_error(aState);
        return 1;
    }
    lua_pushvalue(aState, -1);
    int callbackId = luaL_ref(aState, LUA_REGISTRYINDEX);
    lua_pushinteger(aState, callbackId);
    return 1;
}

int luaApi_dynamo_unregisterCallback(lua_State *aState)
{
    luaL_unref(aState, LUA_REGISTRYINDEX, lua_tointeger(aState, -1));    
    return 0;
}
