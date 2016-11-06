// The MIT License (MIT) 

// Copyright (c) 2013-2016 Rapptz, ThePhD and contributors

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef SOL_5_1_0_H
#define SOL_5_1_0_H

#include "version.hpp"

#if SOL_LUA_VERSION == 501
/* Lua 5.1 */

#include <lua.hpp>
#include <stddef.h>
#include <string.h>
#include <stdio.h>

/* LuaJIT doesn't define these unofficial macros ... */
#if !defined(LUAI_INT32)
#include <limits.h>
#if INT_MAX-20 < 32760
#define LUAI_INT32  long
#define LUAI_UINT32 unsigned long
#elif INT_MAX > 2147483640L
#define LUAI_INT32  int
#define LUAI_UINT32 unsigned int
#else
#error "could not detect suitable lua_Unsigned datatype"
#endif
#endif

/* LuaJIT does not have the updated error codes for thread status/function returns */
#ifndef LUA_ERRGCMM
#define LUA_ERRGCMM (LUA_ERRERR + 1)
#endif // LUA_ERRGCMM

/* LuaJIT does not support continuation contexts / return error codes? */
#ifndef LUA_KCONTEXT
#define LUA_KCONTEXT std::ptrdiff_t
typedef LUA_KCONTEXT lua_KContext;
typedef int(*lua_KFunction) (lua_State *L, int status, lua_KContext ctx);
#endif // LUA_KCONTEXT

#define LUA_OPADD 0
#define LUA_OPSUB 1
#define LUA_OPMUL 2
#define LUA_OPDIV 3
#define LUA_OPMOD 4
#define LUA_OPPOW 5
#define LUA_OPUNM 6
#define LUA_OPEQ 0
#define LUA_OPLT 1
#define LUA_OPLE 2

typedef LUAI_UINT32 lua_Unsigned;

typedef struct luaL_Buffer_52 {
    luaL_Buffer b; /* make incorrect code crash! */
    char *ptr;
    size_t nelems;
    size_t capacity;
    lua_State *L2;
} luaL_Buffer_52;
#define luaL_Buffer luaL_Buffer_52

#define lua_tounsigned(L, i) lua_tounsignedx(L, i, NULL)

#define lua_rawlen(L, i) lua_objlen(L, i)

inline void lua_callk(lua_State *L, int nargs, int nresults, lua_KContext, lua_KFunction) {
    // should probably warn the user of Lua 5.1 that continuation isn't supported...
    lua_call(L, nargs, nresults);
}
inline int lua_pcallk(lua_State *L, int nargs, int nresults, int errfunc, lua_KContext, lua_KFunction) {
    // should probably warn the user of Lua 5.1 that continuation isn't supported...
    return lua_pcall(L, nargs, nresults, errfunc);
}
void lua_arith(lua_State *L, int op);
int lua_compare(lua_State *L, int idx1, int idx2, int op);
void lua_pushunsigned(lua_State *L, lua_Unsigned n);
lua_Unsigned luaL_checkunsigned(lua_State *L, int i);
lua_Unsigned lua_tounsignedx(lua_State *L, int i, int *isnum);
lua_Unsigned luaL_optunsigned(lua_State *L, int i, lua_Unsigned def);
lua_Integer lua_tointegerx(lua_State *L, int i, int *isnum);
void lua_len(lua_State *L, int i);
int luaL_len(lua_State *L, int i);
const char *luaL_tolstring(lua_State *L, int idx, size_t *len);
void luaL_requiref(lua_State *L, char const* modname, lua_CFunction openf, int glb);

#define luaL_buffinit luaL_buffinit_52
void luaL_buffinit(lua_State *L, luaL_Buffer_52 *B);

#define luaL_prepbuffsize luaL_prepbuffsize_52
char *luaL_prepbuffsize(luaL_Buffer_52 *B, size_t s);

#define luaL_addlstring luaL_addlstring_52
void luaL_addlstring(luaL_Buffer_52 *B, const char *s, size_t l);

#define luaL_addvalue luaL_addvalue_52
void luaL_addvalue(luaL_Buffer_52 *B);

#define luaL_pushresult luaL_pushresult_52
void luaL_pushresult(luaL_Buffer_52 *B);

#undef luaL_buffinitsize
#define luaL_buffinitsize(L, B, s) \
  (luaL_buffinit(L, B), luaL_prepbuffsize(B, s))

#undef luaL_prepbuffer
#define luaL_prepbuffer(B) \
  luaL_prepbuffsize(B, LUAL_BUFFERSIZE)

#undef luaL_addchar
#define luaL_addchar(B, c) \
  ((void)((B)->nelems < (B)->capacity || luaL_prepbuffsize(B, 1)), \
   ((B)->ptr[(B)->nelems++] = (c)))

#undef luaL_addsize
#define luaL_addsize(B, s) \
  ((B)->nelems += (s))

#undef luaL_addstring
#define luaL_addstring(B, s) \
  luaL_addlstring(B, s, strlen(s))

#undef luaL_pushresultsize
#define luaL_pushresultsize(B, s) \
  (luaL_addsize(B, s), luaL_pushresult(B))

typedef struct kepler_lua_compat_get_string_view {
	const char *s;
	size_t size;
} kepler_lua_compat_get_string_view;

inline const char* kepler_lua_compat_get_string(lua_State* L, void* ud, size_t* size) {
    kepler_lua_compat_get_string_view* ls = (kepler_lua_compat_get_string_view*) ud;
    (void)L;
    if (ls->size == 0) return NULL;
    *size = ls->size;
    ls->size = 0;
    return ls->s;
}

#if !defined(SOL_LUAJIT) || ((SOL_LUAJIT_VERSION - 20100) <= 0)
// Luajit 2.1.0 has this function already

inline int luaL_loadbufferx(lua_State* L, const char* buff, size_t size, const char* name, const char*) {
    kepler_lua_compat_get_string_view ls;
    ls.s = buff;
    ls.size = size;
    return lua_load(L, kepler_lua_compat_get_string, &ls, name/*, mode*/);
}

#endif // LuaJIT 2.1.x beta and beyond

#endif /* Lua 5.1 */

#endif // SOL_5_1_0_H