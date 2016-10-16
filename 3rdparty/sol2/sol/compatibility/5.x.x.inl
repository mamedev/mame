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

#ifndef SOL_5_X_X_INL
#define SOL_5_X_X_INL

#include "version.hpp"
#include "5.1.0.h"
#include "5.0.0.h"
#include "5.x.x.h"

#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM == 501

#include <errno.h>
#include <string.h>

#define PACKAGE_KEY "_sol.package"

inline int lua_absindex(lua_State *L, int i) {
    if (i < 0 && i > LUA_REGISTRYINDEX)
        i += lua_gettop(L) + 1;
    return i;
}

inline void lua_copy(lua_State *L, int from, int to) {
    int abs_to = lua_absindex(L, to);
    luaL_checkstack(L, 1, "not enough stack slots");
    lua_pushvalue(L, from);
    lua_replace(L, abs_to);
}

inline void lua_rawgetp(lua_State *L, int i, const void *p) {
    int abs_i = lua_absindex(L, i);
    lua_pushlightuserdata(L, (void*)p);
    lua_rawget(L, abs_i);
}

inline void lua_rawsetp(lua_State *L, int i, const void *p) {
    int abs_i = lua_absindex(L, i);
    luaL_checkstack(L, 1, "not enough stack slots");
    lua_pushlightuserdata(L, (void*)p);
    lua_insert(L, -2);
    lua_rawset(L, abs_i);
}

inline void *luaL_testudata(lua_State *L, int i, const char *tname) {
    void *p = lua_touserdata(L, i);
    luaL_checkstack(L, 2, "not enough stack slots");
    if (p == NULL || !lua_getmetatable(L, i))
        return NULL;
    else {
        int res = 0;
        luaL_getmetatable(L, tname);
        res = lua_rawequal(L, -1, -2);
        lua_pop(L, 2);
        if (!res)
            p = NULL;
    }
    return p;
}

inline lua_Number lua_tonumberx(lua_State *L, int i, int *isnum) {
    lua_Number n = lua_tonumber(L, i);
    if (isnum != NULL) {
        *isnum = (n != 0 || lua_isnumber(L, i));
    }
    return n;
}

inline static void push_package_table(lua_State *L) {
    lua_pushliteral(L, PACKAGE_KEY);
    lua_rawget(L, LUA_REGISTRYINDEX);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        /* try to get package table from globals */
        lua_pushliteral(L, "package");
        lua_rawget(L, LUA_GLOBALSINDEX);
        if (lua_istable(L, -1)) {
            lua_pushliteral(L, PACKAGE_KEY);
            lua_pushvalue(L, -2);
            lua_rawset(L, LUA_REGISTRYINDEX);
        }
    }
}

inline void lua_getuservalue(lua_State *L, int i) {
    luaL_checktype(L, i, LUA_TUSERDATA);
    luaL_checkstack(L, 2, "not enough stack slots");
    lua_getfenv(L, i);
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    if (lua_rawequal(L, -1, -2)) {
        lua_pop(L, 1);
        lua_pushnil(L);
        lua_replace(L, -2);
    }
    else {
        lua_pop(L, 1);
        push_package_table(L);
        if (lua_rawequal(L, -1, -2)) {
            lua_pop(L, 1);
            lua_pushnil(L);
            lua_replace(L, -2);
        }
        else
            lua_pop(L, 1);
    }
}

inline void lua_setuservalue(lua_State *L, int i) {
    luaL_checktype(L, i, LUA_TUSERDATA);
    if (lua_isnil(L, -1)) {
        luaL_checkstack(L, 1, "not enough stack slots");
        lua_pushvalue(L, LUA_GLOBALSINDEX);
        lua_replace(L, -2);
    }
    lua_setfenv(L, i);
}

/*
** Adapted from Lua 5.2.0
*/
inline void luaL_setfuncs(lua_State *L, const luaL_Reg *l, int nup) {
    luaL_checkstack(L, nup + 1, "too many upvalues");
    for (; l->name != NULL; l++) {  /* fill the table with given functions */
        int i;
        lua_pushstring(L, l->name);
        for (i = 0; i < nup; i++)  /* copy upvalues to the top */
            lua_pushvalue(L, -(nup + 1));
        lua_pushcclosure(L, l->func, nup);  /* closure with those upvalues */
        lua_settable(L, -(nup + 3)); /* table must be below the upvalues, the name and the closure */
    }
    lua_pop(L, nup);  /* remove upvalues */
}

inline void luaL_setmetatable(lua_State *L, const char *tname) {
    luaL_checkstack(L, 1, "not enough stack slots");
    luaL_getmetatable(L, tname);
    lua_setmetatable(L, -2);
}

inline int luaL_getsubtable(lua_State *L, int i, const char *name) {
    int abs_i = lua_absindex(L, i);
    luaL_checkstack(L, 3, "not enough stack slots");
    lua_pushstring(L, name);
    lua_gettable(L, abs_i);
    if (lua_istable(L, -1))
        return 1;
    lua_pop(L, 1);
    lua_newtable(L);
    lua_pushstring(L, name);
    lua_pushvalue(L, -2);
    lua_settable(L, abs_i);
    return 0;
}

#ifndef SOL_LUAJIT
inline static int countlevels(lua_State *L) {
    lua_Debug ar;
    int li = 1, le = 1;
    /* find an upper bound */
    while (lua_getstack(L, le, &ar)) { li = le; le *= 2; }
    /* do a binary search */
    while (li < le) {
        int m = (li + le) / 2;
        if (lua_getstack(L, m, &ar)) li = m + 1;
        else le = m;
    }
    return le - 1;
}

inline static int findfield(lua_State *L, int objidx, int level) {
    if (level == 0 || !lua_istable(L, -1))
        return 0;  /* not found */
    lua_pushnil(L);  /* start 'next' loop */
    while (lua_next(L, -2)) {  /* for each pair in table */
        if (lua_type(L, -2) == LUA_TSTRING) {  /* ignore non-string keys */
            if (lua_rawequal(L, objidx, -1)) {  /* found object? */
                lua_pop(L, 1);  /* remove value (but keep name) */
                return 1;
            }
            else if (findfield(L, objidx, level - 1)) {  /* try recursively */
                lua_remove(L, -2);  /* remove table (but keep name) */
                lua_pushliteral(L, ".");
                lua_insert(L, -2);  /* place '.' between the two names */
                lua_concat(L, 3);
                return 1;
            }
        }
        lua_pop(L, 1);  /* remove value */
    }
    return 0;  /* not found */
}

inline static int pushglobalfuncname(lua_State *L, lua_Debug *ar) {
    int top = lua_gettop(L);
    lua_getinfo(L, "f", ar);  /* push function */
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    if (findfield(L, top + 1, 2)) {
        lua_copy(L, -1, top + 1);  /* move name to proper place */
        lua_pop(L, 2);  /* remove pushed values */
        return 1;
    }
    else {
        lua_settop(L, top);  /* remove function and global table */
        return 0;
    }
}

inline static void pushfuncname(lua_State *L, lua_Debug *ar) {
    if (*ar->namewhat != '\0')  /* is there a name? */
        lua_pushfstring(L, "function " LUA_QS, ar->name);
    else if (*ar->what == 'm')  /* main? */
        lua_pushliteral(L, "main chunk");
    else if (*ar->what == 'C') {
        if (pushglobalfuncname(L, ar)) {
            lua_pushfstring(L, "function " LUA_QS, lua_tostring(L, -1));
            lua_remove(L, -2);  /* remove name */
        }
        else
            lua_pushliteral(L, "?");
    }
    else
        lua_pushfstring(L, "function <%s:%d>", ar->short_src, ar->linedefined);
}

#define LEVELS1 12  /* size of the first part of the stack */
#define LEVELS2 10  /* size of the second part of the stack */

inline void luaL_traceback(lua_State *L, lua_State *L1,
    const char *msg, int level) {
    lua_Debug ar;
    int top = lua_gettop(L);
    int numlevels = countlevels(L1);
    int mark = (numlevels > LEVELS1 + LEVELS2) ? LEVELS1 : 0;
    if (msg) lua_pushfstring(L, "%s\n", msg);
    lua_pushliteral(L, "stack traceback:");
    while (lua_getstack(L1, level++, &ar)) {
        if (level == mark) {  /* too many levels? */
            lua_pushliteral(L, "\n\t...");  /* add a '...' */
            level = numlevels - LEVELS2;  /* and skip to last ones */
        }
        else {
            lua_getinfo(L1, "Slnt", &ar);
            lua_pushfstring(L, "\n\t%s:", ar.short_src);
            if (ar.currentline > 0)
                lua_pushfstring(L, "%d:", ar.currentline);
            lua_pushliteral(L, " in ");
            pushfuncname(L, &ar);
            lua_concat(L, lua_gettop(L) - top);
        }
    }
    lua_concat(L, lua_gettop(L) - top);
}
#endif

inline const lua_Number *lua_version(lua_State *L) {
    static const lua_Number version = LUA_VERSION_NUM;
    if (L == NULL) return &version;
    // TODO: wonky hacks to get at the inside of the incomplete type lua_State?
    //else return L->l_G->version;
    else return &version;
}

inline static void luaL_checkversion_(lua_State *L, lua_Number ver) {
    const lua_Number* v = lua_version(L);
    if (v != lua_version(NULL))
        luaL_error(L, "multiple Lua VMs detected");
    else if (*v != ver)
        luaL_error(L, "version mismatch: app. needs %f, Lua core provides %f",
            ver, *v);
    /* check conversions number -> integer types */
    lua_pushnumber(L, -(lua_Number)0x1234);
    if (lua_tointeger(L, -1) != -0x1234 ||
        lua_tounsigned(L, -1) != (lua_Unsigned)-0x1234)
        luaL_error(L, "bad conversion number->int;"
            " must recompile Lua with proper settings");
    lua_pop(L, 1);
}

inline void luaL_checkversion(lua_State* L) {
    luaL_checkversion_(L, LUA_VERSION_NUM);
}

#ifndef SOL_LUAJIT
inline int luaL_fileresult(lua_State *L, int stat, const char *fname) {
    int en = errno;  /* calls to Lua API may change this value */
    if (stat) {
        lua_pushboolean(L, 1);
        return 1;
    }
    else {
        char buf[1024];
#ifdef __GLIBC__
        strerror_r(en, buf, 1024);
#else
        strerror_s(buf, 1024, en);
#endif
        lua_pushnil(L);
        if (fname)
            lua_pushfstring(L, "%s: %s", fname, buf);
        else
            lua_pushstring(L, buf);
        lua_pushnumber(L, (lua_Number)en);
        return 3;
    }
}
#endif // luajit
#endif // Lua 5.0 or Lua 5.1


#if SOL_LUA_VERSION == 501
#include <limits.h>

typedef LUAI_INT32 LUA_INT32;

/********************************************************************/
/*                    extract of 5.2's luaconf.h                    */
/*  detects proper defines for faster unsigned<->number conversion  */
/*           see copyright notice at the end of this file           */
/********************************************************************/

#if !defined(LUA_ANSI) && defined(_WIN32) && !defined(_WIN32_WCE)
#define LUA_WIN        /* enable goodies for regular Windows platforms */
#endif


#if defined(LUA_NUMBER_DOUBLE) && !defined(LUA_ANSI)    /* { */

/* Microsoft compiler on a Pentium (32 bit) ? */
#if defined(LUA_WIN) && defined(_MSC_VER) && defined(_M_IX86)    /* { */

#define LUA_MSASMTRICK
#define LUA_IEEEENDIAN        0
#define LUA_NANTRICK

/* pentium 32 bits? */
#elif defined(__i386__) || defined(__i386) || defined(__X86__) /* }{ */

#define LUA_IEEE754TRICK
#define LUA_IEEELL
#define LUA_IEEEENDIAN        0
#define LUA_NANTRICK

/* pentium 64 bits? */
#elif defined(__x86_64)                        /* }{ */

#define LUA_IEEE754TRICK
#define LUA_IEEEENDIAN        0

#elif defined(__POWERPC__) || defined(__ppc__)            /* }{ */

#define LUA_IEEE754TRICK
#define LUA_IEEEENDIAN        1

#else                                /* }{ */

/* assume IEEE754 and a 32-bit integer type */
#define LUA_IEEE754TRICK

#endif                                /* } */

#endif                            /* } */


/********************************************************************/
/*                    extract of 5.2's llimits.h                    */
/*       gives us lua_number2unsigned and lua_unsigned2number       */
/*           see copyright notice just below this one here          */
/********************************************************************/

/*********************************************************************
* This file contains parts of Lua 5.2's source code:
*
* Copyright (C) 1994-2013 Lua.org, PUC-Rio.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*********************************************************************/

#if defined(MS_ASMTRICK) || defined(LUA_MSASMTRICK)    /* { */
/* trick with Microsoft assembler for X86 */

#define lua_number2unsigned(i,n)  \
  {__int64 l; __asm {__asm fld n   __asm fistp l} i = (unsigned int)l;}


#elif defined(LUA_IEEE754TRICK)        /* }{ */
/* the next trick should work on any machine using IEEE754 with
a 32-bit int type */

union compat52_luai_Cast { double l_d; LUA_INT32 l_p[2]; };

#if !defined(LUA_IEEEENDIAN)    /* { */
#define LUAI_EXTRAIEEE    \
  static const union compat52_luai_Cast ieeeendian = {-(33.0 + 6755399441055744.0)};
#define LUA_IEEEENDIANLOC    (ieeeendian.l_p[1] == 33)
#else
#define LUA_IEEEENDIANLOC    LUA_IEEEENDIAN
#define LUAI_EXTRAIEEE        /* empty */
#endif                /* } */

#define lua_number2int32(i,n,t) \
  { LUAI_EXTRAIEEE \
    volatile union compat52_luai_Cast u; u.l_d = (n) + 6755399441055744.0; \
    (i) = (t)u.l_p[LUA_IEEEENDIANLOC]; }

#define lua_number2unsigned(i,n)    lua_number2int32(i, n, lua_Unsigned)

#endif                /* } */


/* the following definitions always work, but may be slow */

#if !defined(lua_number2unsigned)    /* { */
/* the following definition assures proper modulo behavior */
#if defined(LUA_NUMBER_DOUBLE) || defined(LUA_NUMBER_FLOAT)
#include <math.h>
#define SUPUNSIGNED    ((lua_Number)(~(lua_Unsigned)0) + 1)
#define lua_number2unsigned(i,n)  \
    ((i)=(lua_Unsigned)((n) - floor((n)/SUPUNSIGNED)*SUPUNSIGNED))
#else
#define lua_number2unsigned(i,n)    ((i)=(lua_Unsigned)(n))
#endif
#endif                /* } */


#if !defined(lua_unsigned2number)
/* on several machines, coercion from unsigned to double is slow,
so it may be worth to avoid */
#define lua_unsigned2number(u)  \
    (((u) <= (lua_Unsigned)INT_MAX) ? (lua_Number)(int)(u) : (lua_Number)(u))
#endif

/********************************************************************/

inline static void compat52_call_lua(lua_State *L, char const code[], size_t len,
    int nargs, int nret) {
    lua_rawgetp(L, LUA_REGISTRYINDEX, (void*)code);
    if (lua_type(L, -1) != LUA_TFUNCTION) {
        lua_pop(L, 1);
        if (luaL_loadbuffer(L, code, len, "=none"))
            lua_error(L);
        lua_pushvalue(L, -1);
        lua_rawsetp(L, LUA_REGISTRYINDEX, (void*)code);
    }
    lua_insert(L, -nargs - 1);
    lua_call(L, nargs, nret);
}

static const char compat52_arith_code[] = {
    "local op,a,b=...\n"
    "if op==0 then return a+b\n"
    "elseif op==1 then return a-b\n"
    "elseif op==2 then return a*b\n"
    "elseif op==3 then return a/b\n"
    "elseif op==4 then return a%b\n"
    "elseif op==5 then return a^b\n"
    "elseif op==6 then return -a\n"
    "end\n"
};

inline void lua_arith(lua_State *L, int op) {
    if (op < LUA_OPADD || op > LUA_OPUNM)
        luaL_error(L, "invalid 'op' argument for lua_arith");
    luaL_checkstack(L, 5, "not enough stack slots");
    if (op == LUA_OPUNM)
        lua_pushvalue(L, -1);
    lua_pushnumber(L, op);
    lua_insert(L, -3);
    compat52_call_lua(L, compat52_arith_code,
        sizeof(compat52_arith_code) - 1, 3, 1);
}

static const char compat52_compare_code[] = {
    "local a,b=...\n"
    "return a<=b\n"
};

inline int lua_compare(lua_State *L, int idx1, int idx2, int op) {
    int result = 0;
    switch (op) {
    case LUA_OPEQ:
        return lua_equal(L, idx1, idx2);
    case LUA_OPLT:
        return lua_lessthan(L, idx1, idx2);
    case LUA_OPLE:
        luaL_checkstack(L, 5, "not enough stack slots");
        idx1 = lua_absindex(L, idx1);
        idx2 = lua_absindex(L, idx2);
        lua_pushvalue(L, idx1);
        lua_pushvalue(L, idx2);
        compat52_call_lua(L, compat52_compare_code,
            sizeof(compat52_compare_code) - 1, 2, 1);
        result = lua_toboolean(L, -1);
        lua_pop(L, 1);
        return result;
    default:
        luaL_error(L, "invalid 'op' argument for lua_compare");
    }
    return 0;
}

inline void lua_pushunsigned(lua_State *L, lua_Unsigned n) {
    lua_pushnumber(L, lua_unsigned2number(n));
}

inline lua_Unsigned luaL_checkunsigned(lua_State *L, int i) {
    lua_Unsigned result;
    lua_Number n = lua_tonumber(L, i);
    if (n == 0 && !lua_isnumber(L, i))
        luaL_checktype(L, i, LUA_TNUMBER);
    lua_number2unsigned(result, n);
    return result;
}

inline lua_Unsigned lua_tounsignedx(lua_State *L, int i, int *isnum) {
    lua_Unsigned result;
    lua_Number n = lua_tonumberx(L, i, isnum);
    lua_number2unsigned(result, n);
    return result;
}

inline lua_Unsigned luaL_optunsigned(lua_State *L, int i, lua_Unsigned def) {
    return luaL_opt(L, luaL_checkunsigned, i, def);
}

inline lua_Integer lua_tointegerx(lua_State *L, int i, int *isnum) {
    lua_Integer n = lua_tointeger(L, i);
    if (isnum != NULL) {
        *isnum = (n != 0 || lua_isnumber(L, i));
    }
    return n;
}

inline void lua_len(lua_State *L, int i) {
    switch (lua_type(L, i)) {
    case LUA_TSTRING: /* fall through */
    case LUA_TTABLE:
        if (!luaL_callmeta(L, i, "__len"))
            lua_pushnumber(L, (int)lua_objlen(L, i));
        break;
    case LUA_TUSERDATA:
        if (luaL_callmeta(L, i, "__len"))
            break;
        /* maybe fall through */
    default:
        luaL_error(L, "attempt to get length of a %s value",
            lua_typename(L, lua_type(L, i)));
    }
}

inline int luaL_len(lua_State *L, int i) {
    int res = 0, isnum = 0;
    luaL_checkstack(L, 1, "not enough stack slots");
    lua_len(L, i);
    res = (int)lua_tointegerx(L, -1, &isnum);
    lua_pop(L, 1);
    if (!isnum)
        luaL_error(L, "object length is not a number");
    return res;
}

inline const char *luaL_tolstring(lua_State *L, int idx, size_t *len) {
    if (!luaL_callmeta(L, idx, "__tostring")) {
        int t = lua_type(L, idx);
        switch (t) {
        case LUA_TNIL:
            lua_pushliteral(L, "nil");
            break;
        case LUA_TSTRING:
        case LUA_TNUMBER:
            lua_pushvalue(L, idx);
            break;
        case LUA_TBOOLEAN:
            if (lua_toboolean(L, idx))
                lua_pushliteral(L, "true");
            else
                lua_pushliteral(L, "false");
            break;
        default:
            lua_pushfstring(L, "%s: %p", lua_typename(L, t),
                lua_topointer(L, idx));
            break;
        }
    }
    return lua_tolstring(L, -1, len);
}

inline void luaL_requiref(lua_State *L, char const* modname,
    lua_CFunction openf, int glb) {
    luaL_checkstack(L, 3, "not enough stack slots");
    lua_pushcfunction(L, openf);
    lua_pushstring(L, modname);
    lua_call(L, 1, 1);
    lua_getglobal(L, "package");
    if (lua_istable(L, -1) == 0) {
        lua_pop(L, 1);
        lua_createtable(L, 0, 16);
        lua_setglobal(L, "package");
        lua_getglobal(L, "package");
    }
    lua_getfield(L, -1, "loaded");
    if (lua_istable(L, -1) == 0) {
        lua_pop(L, 1);
        lua_createtable(L, 0, 1);
        lua_setfield(L, -2, "loaded");
        lua_getfield(L, -1, "loaded");
    }
    lua_replace(L, -2);
    lua_pushvalue(L, -2);
    lua_setfield(L, -2, modname);
    lua_pop(L, 1);
    if (glb) {
        lua_pushvalue(L, -1);
        lua_setglobal(L, modname);
    }
}

inline void luaL_buffinit(lua_State *L, luaL_Buffer_52 *B) {
    /* make it crash if used via pointer to a 5.1-style luaL_Buffer */
    B->b.p = NULL;
    B->b.L = NULL;
    B->b.lvl = 0;
    /* reuse the buffer from the 5.1-style luaL_Buffer though! */
    B->ptr = B->b.buffer;
    B->capacity = LUAL_BUFFERSIZE;
    B->nelems = 0;
    B->L2 = L;
}

inline char *luaL_prepbuffsize(luaL_Buffer_52 *B, size_t s) {
    if (B->capacity - B->nelems < s) { /* needs to grow */
        char* newptr = NULL;
        size_t newcap = B->capacity * 2;
        if (newcap - B->nelems < s)
            newcap = B->nelems + s;
        if (newcap < B->capacity) /* overflow */
            luaL_error(B->L2, "buffer too large");
        newptr = (char*)lua_newuserdata(B->L2, newcap);
        memcpy(newptr, B->ptr, B->nelems);
        if (B->ptr != B->b.buffer)
            lua_replace(B->L2, -2); /* remove old buffer */
        B->ptr = newptr;
        B->capacity = newcap;
    }
    return B->ptr + B->nelems;
}

inline void luaL_addlstring(luaL_Buffer_52 *B, const char *s, size_t l) {
    memcpy(luaL_prepbuffsize(B, l), s, l);
    luaL_addsize(B, l);
}

inline void luaL_addvalue(luaL_Buffer_52 *B) {
    size_t len = 0;
    const char *s = lua_tolstring(B->L2, -1, &len);
    if (!s)
        luaL_error(B->L2, "cannot convert value to string");
    if (B->ptr != B->b.buffer)
        lua_insert(B->L2, -2); /* userdata buffer must be at stack top */
    luaL_addlstring(B, s, len);
    lua_remove(B->L2, B->ptr != B->b.buffer ? -2 : -1);
}

inline void luaL_pushresult(luaL_Buffer_52 *B) {
    lua_pushlstring(B->L2, B->ptr, B->nelems);
    if (B->ptr != B->b.buffer)
        lua_replace(B->L2, -2); /* remove userdata buffer */
}

#endif /* SOL_LUA_VERSION == 501 */

#endif // SOL_5_X_X_INL
