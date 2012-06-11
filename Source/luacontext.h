/*!
	@header Lua Context
	@abstract
	@discussion Tools for managing a lua context
*/

#ifndef _LUACTX_H_
#define _LUACTX_H_

#include "object.h"
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

extern Class_t Class_LuaContext;
/*!
	Encapsulates a LuaJIT context. (Essentially just provides memory management)
*/
typedef struct _LuaContext {
    OBJ_GUTS
    lua_State *luaState;
} LuaContext_t;

extern LuaContext_t *GlobalLuaContext;

/*!
 	Inits the global context
*/
extern void luaCtx_init();

/*!
    Destroys the global context.
*/
extern void luaCtx_teardown();

/*!
	Creates a lua context
*/
extern LuaContext_t *luaCtx_createContext();

/*!
	Gets a named global variable and pushes it on the lua stack.
*/
extern void luaCtx_getglobal(LuaContext_t *aCtx, const char *k);
/*!
	Pops 'n' values off the top of the lua stack.
*/
extern void luaCtx_pop(LuaContext_t *aCtx, int n);
/*!
	Creates a table at the top of the lua stack.
*/
extern void luaCtx_createtable(LuaContext_t *aCtx, int narr, int nrec);
/*!
	Gets field 'k' in the table at stack index 'idx'.
*/
extern void luaCtx_getfield(LuaContext_t *aCtx, int idx, const char *k);
/*!
	Sets the field 'k' in the table at stack index 'idx' to the value at stack index -1.
*/
extern void luaCtx_setfield(LuaContext_t *aCtx, int idx, const char *k);
/*!
	Wrapper for pcall that handles errors.
*/
extern bool luaCtx_pcall(LuaContext_t *aCtx, int nargs, int nrseults, int errfunc);
/*!
	Loads and executes the file at the given path.
*/
extern bool luaCtx_executeFile(LuaContext_t *aCtx, const char *aPath);
/*!
	Executes the given lua string.
*/
extern bool luaCtx_executeString(LuaContext_t *aCtx, const char *aScript);
/*!
	Adds a path to the global lua search path (Used by require()).
*/
extern bool luaCtx_addSearchPath(LuaContext_t *aCtx, const char *aPath);
/*!
	 Pushes the value at 'idx' of the lua stack onto the top of the stack.
*/
extern void luaCtx_pushvalue(LuaContext_t *aCtx, int idx);
/*!
	 Pushes nil on the lua stack.
*/
extern void luaCtx_pushnil(LuaContext_t *aCtx);
/*!
	 Pushes a number(float) on the lua stack.
*/
extern void luaCtx_pushnumber(LuaContext_t *aCtx, float n);
/*!
	 Pushes an integer on the lua stack.
*/
extern void luaCtx_pushinteger(LuaContext_t *aCtx, int n);
/*!
	 Pushes a string on the lua stack.
*/
extern void luaCtx_pushstring(LuaContext_t *aCtx, const char *s);
/*!
	 Pushes a boolean on the lua stack.
*/
extern void luaCtx_pushboolean(LuaContext_t *aCtx, int b);
/*!
	 Pushes a pointer as light userdata object onto the lua stack.
*/
void luaCtx_pushlightuserdata(LuaContext_t *aCtx, void *ptr);
/*!
	Pushes a registered script handler onto the lua stack.
*/
extern bool luaCtx_pushScriptHandler(LuaContext_t *aCtx, int aRefId);
/*!
	Deletes a script handler from the lua registry.
*/
extern void luaCtx_unregisterScriptHandler(LuaContext_t *aCtx, int aRefId);
/*!
	Concatenates the 'n' top elements of the lua stack.
*/
extern void luaCtx_concat(LuaContext_t *aCtx, int n);
#endif
