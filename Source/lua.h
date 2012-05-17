// Maintains a LuaJIT context and provides some convenience functions for common cases

#ifndef _LUACTX_H_
#define _LUACTX_H_

#include "object.h"
#import <lua.h>
#import <lauxlib.h>
#import <lualib.h>

extern Class_t Class_LuaContext;
typedef struct _LuaContext {
    OBJ_GUTS
    lua_State *luaState;
} LuaContext_t;

LuaContext_t *luaCtx_createContext();
// Wrapper for pcall that handles errors
extern bool luaCtx_pcall(LuaContext_t *aCtx, int nargs, int nrseults, int errfunc);
extern bool luaCtx_executeFile(LuaContext_t *aCtx, const char *aPath);
extern bool luaCtx_executeString(LuaContext_t *aCtx, const char *aScript);
extern bool luaCtx_addSearchPath(LuaContext_t *aCtx, const char *aPath);
#endif