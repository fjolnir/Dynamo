#include "luacontext.h"
#include "util.h"
#include <string.h>
#include <stdlib.h>

static void luaCtx_destroy(LuaContext_t *aCtx);

Class_t Class_LuaContext = {
    "LuaContext",
    sizeof(LuaContext_t),
    (Obj_destructor_t)&luaCtx_destroy
};

LuaContext_t *luaCtx_createContext()
{
    LuaContext_t *out = obj_create_autoreleased(&Class_LuaContext);

    out->luaState = lua_open();
    luaL_openlibs(out->luaState);
    
    return out;
}

static void luaCtx_destroy(LuaContext_t *aCtx)
{
    lua_close(aCtx->luaState);
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
