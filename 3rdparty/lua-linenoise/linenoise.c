/* vim:sts=4 sw=4 expandtab
 */

/*
* Copyright (c) 2011-2015 Rob Hoelz <rob@hoelz.ro>
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is furnished
* to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
* FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
* COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
* IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <lua.h>
#include <lauxlib.h>
#include <stdlib.h>
#include "linenoise.h"

#define LN_COMPLETION_TYPE "linenoiseCompletions*"

static int completion_func_ref;
static lua_State *completion_state;

static int handle_ln_error(lua_State *L)
{
    lua_pushnil(L);
    return 1;
}

static int handle_ln_ok(lua_State *L)
{
    lua_pushboolean(L, 1);
    return 1;
}

static void completion_callback_wrapper(const char *line, linenoiseCompletions *completions, int pos)
{
    lua_State *L = completion_state;

    lua_rawgeti(L, LUA_REGISTRYINDEX, completion_func_ref);
    *((linenoiseCompletions **) lua_newuserdata(L, sizeof(linenoiseCompletions *))) = completions;
    luaL_getmetatable(L, LN_COMPLETION_TYPE);
    lua_setmetatable(L, -2);

    lua_pushstring(L, line);
    lua_pushinteger(L, pos);

    lua_pcall(L, 3, 0, 0);
}

static int l_linenoise(lua_State *L)
{
    const char *prompt = luaL_checkstring(L, 1);
    char *line;

    completion_state = L;
    line = linenoise(prompt);
    completion_state = NULL;

    if(! line) {
        return handle_ln_error(L);
    }
    lua_pushstring(L, line);
    free(line);
    return 1;
}

static int lines_next(lua_State *L)
{
    lua_pushcfunction(L, l_linenoise);
    lua_pushvalue(L, lua_upvalueindex(1));
    lua_call(L, 1, 1);
    return 1;
}

static int l_lines(lua_State *L)
{
    luaL_checkstring(L, 1);
    lua_pushcclosure(L, lines_next, 1);
    return 1;
}

static int l_historyadd(lua_State *L)
{
    const char *line = luaL_checkstring(L, 1);

    if(! linenoiseHistoryAdd(line)) {
        return handle_ln_error(L);
    }

    return handle_ln_ok(L);
}

static int l_preloadbuffer(lua_State *L)
{
    const char *line = luaL_checkstring(L, 1);
    linenoisePreloadBuffer(line);

    return handle_ln_ok(L);
}

static int l_historysetmaxlen(lua_State *L)
{
    int len = luaL_checkinteger(L, 1);

    if(! linenoiseHistorySetMaxLen(len)) {
        return handle_ln_error(L);
    }

    return handle_ln_ok(L);
}

static int l_historysave(lua_State *L)
{
    const char *filename = luaL_checkstring(L, 1);

    if(linenoiseHistorySave((char *) filename) < 0) {
        return handle_ln_error(L);
    }
    return handle_ln_ok(L);
}

static int l_historyload(lua_State *L)
{
    const char *filename = luaL_checkstring(L, 1);

    if(linenoiseHistoryLoad((char *) filename) < 0) {
        return handle_ln_error(L);
    }
    return handle_ln_ok(L);
}

static int l_clearscreen(lua_State *L)
{
    linenoiseClearScreen();
    return handle_ln_ok(L);
}

static int l_setcompletion(lua_State *L)
{
    luaL_checktype(L, 1, LUA_TFUNCTION);

    lua_pushvalue(L, 1);
    completion_func_ref = luaL_ref(L, LUA_REGISTRYINDEX);
    linenoiseSetCompletionCallback(completion_callback_wrapper);

    return handle_ln_ok(L);
}

static int l_addcompletion(lua_State *L)
{
    linenoiseCompletions *completions = *((linenoiseCompletions **) luaL_checkudata(L, 1, LN_COMPLETION_TYPE));
    const char *entry                 = luaL_checkstring(L, 2);
    int pos                           = luaL_checkinteger(L, 3);

    linenoiseAddCompletion(completions, (char *) entry, pos);

    return handle_ln_ok(L);
}

static int l_refresh(lua_State *L)
{
    linenoiseRefresh();
    return handle_ln_ok(L);
}

static int l_historyget(lua_State *L)
{
    int len, i;
    char **history = linenoiseHistory(&len);
    lua_newtable(L);
    for(i = 0; i < len; i++)
    {
        lua_pushstring(L, history[i]);
        lua_rawseti(L, -2, i + 1);
    }
    return 1;
}

luaL_Reg linenoise_funcs[] = {
    { "linenoise", l_linenoise },
    { "historyadd", l_historyadd },
    { "historysetmaxlen", l_historysetmaxlen },
    { "historysave", l_historysave },
    { "historyload", l_historyload },
    { "historyget", l_historyget },
    { "clearscreen", l_clearscreen },
    { "setcompletion", l_setcompletion},
    { "addcompletion", l_addcompletion },
    { "preload", l_preloadbuffer },
    { "refresh", l_refresh },

    /* Aliases for more consistent function names */
    { "addhistory", l_historyadd },
    { "sethistorymaxlen", l_historysetmaxlen },
    { "savehistory", l_historysave },
    { "loadhistory", l_historyload },

    { "line", l_linenoise },
    { "lines", l_lines },

    { NULL, NULL }
};

int luaopen_linenoise(lua_State *L)
{
    lua_newtable(L);

    luaL_newmetatable(L, LN_COMPLETION_TYPE);
    lua_pushboolean(L, 0);
    lua_setfield(L, -2, "__metatable");
    lua_pop(L, 1);
#if LUA_VERSION_NUM > 501
    luaL_setfuncs(L,linenoise_funcs,0);
#else
    luaL_register(L, NULL, linenoise_funcs);
#endif
    return 1;
}
