/*!
	@header Lua Context
	@abstract
	@discussion Tools for managing a lua context
*/

#ifndef _LUACTX_H_
#define _LUACTX_H_

#include "object.h"
#import "lua.h"
#import "lauxlib.h"
#import "lualib.h"

extern Class_t Class_LuaContext;
/*!
	Encapsulates a LuaJIT context. (Essentially just provides memory management)
*/
typedef struct _LuaContext {
    OBJ_GUTS
    lua_State *luaState;
} LuaContext_t;

/*!
	Creates a lua context
*/
LuaContext_t *luaCtx_createContext();
/*!
	Wrapper for pcall that handles errors
*/
extern bool luaCtx_pcall(LuaContext_t *aCtx, int nargs, int nrseults, int errfunc);
/*!
	Loads and executes the file at the given path
*/
extern bool luaCtx_executeFile(LuaContext_t *aCtx, const char *aPath);
/*!
	Executes the given lua string.
*/
extern bool luaCtx_executeString(LuaContext_t *aCtx, const char *aScript);
/*!
	Adds a path to the global lua search path (Used by require())
*/
extern bool luaCtx_addSearchPath(LuaContext_t *aCtx, const char *aPath);
#endif
