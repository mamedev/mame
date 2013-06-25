/*
 * This source file is part of libRocket, the HTML/CSS Interface Middleware
 *
 * For the latest information, see http://www.librocket.com
 *
 * Copyright (c) 2008-2010 CodePoint Ltd, Shift Technology Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
 
#include "precompiled.h"
#include "Log.h"
#include <Rocket/Core/Log.h>
#include <Rocket/Core/String.h>
#include <Rocket/Core/StringUtilities.h>


namespace Rocket {
namespace Core {
namespace Lua {

template<> void ExtraInit<Log>(lua_State* L, int metatable_index)
{
    //due to they way that LuaType::Register is made, we know that the method table is at the index
    //directly below the metatable
    int method_index = metatable_index - 1;

    lua_pushcfunction(L,LogMessage);
    lua_setfield(L,method_index, "Message");

    //construct the "logtype" table, so that we can use the Rocket::Core::Log::Type enum like Log.logtype.always in Lua for Log::LT_ALWAYS
    lua_newtable(L);
    int logtype = lua_gettop(L);
    lua_pushvalue(L,-1); //copy of the new table, so that the logtype index will stay valid
    lua_setfield(L,method_index,"logtype");

    lua_pushinteger(L,(int)Log::LT_ALWAYS);
    lua_setfield(L,logtype,"always");

    lua_pushinteger(L,(int)Log::LT_ERROR);
    lua_setfield(L,logtype,"error");

    lua_pushinteger(L,(int)Log::LT_WARNING);
    lua_setfield(L,logtype,"warning");

    lua_pushinteger(L,(int)Log::LT_INFO);
    lua_setfield(L,logtype,"info");

    lua_pushinteger(L,(int)Log::LT_DEBUG);
    lua_setfield(L,logtype,"debug");

    lua_pop(L,1); //pop the logtype table
    return;
}

int LogMessage(lua_State* L)
{
    Log::Type type = Log::Type((int)luaL_checkinteger(L,1));
    const char* str = luaL_checkstring(L,2);
    
    Log::Message(type, str);
    return 0;
}

RegType<Log> LogMethods[] =
{
    { NULL, NULL },
};

luaL_reg LogGetters[] =
{
    { NULL, NULL },
};

luaL_reg LogSetters[] =
{
    { NULL, NULL },
};

LUACORETYPEDEFINE(Log,false)
}
}
}