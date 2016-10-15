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

#ifndef SOL_5_X_X_H
#define SOL_5_X_X_H

#include "version.hpp"

#if SOL_LUA_VERSION < 502

#define LUA_RIDX_GLOBALS LUA_GLOBALSINDEX

#define LUA_OK 0

#define lua_pushglobaltable(L) \
  lua_pushvalue(L, LUA_GLOBALSINDEX)

#define luaL_newlib(L, l) \
  (lua_newtable((L)),luaL_setfuncs((L), (l), 0))

void luaL_checkversion(lua_State *L);

int lua_absindex(lua_State *L, int i);
void lua_copy(lua_State *L, int from, int to);
void lua_rawgetp(lua_State *L, int i, const void *p);
void lua_rawsetp(lua_State *L, int i, const void *p);
void *luaL_testudata(lua_State *L, int i, const char *tname);
lua_Number lua_tonumberx(lua_State *L, int i, int *isnum);
void lua_getuservalue(lua_State *L, int i);
void lua_setuservalue(lua_State *L, int i);
void luaL_setfuncs(lua_State *L, const luaL_Reg *l, int nup);
void luaL_setmetatable(lua_State *L, const char *tname);
int luaL_getsubtable(lua_State *L, int i, const char *name);
void luaL_traceback(lua_State *L, lua_State *L1, const char *msg, int level);
int luaL_fileresult(lua_State *L, int stat, const char *fname);

#endif // Lua 5.1 and below

#endif // SOL_5_X_X_H
