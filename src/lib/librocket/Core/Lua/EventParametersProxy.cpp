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
#include "EventParametersProxy.h"
#include <Rocket/Core/Lua/Utilities.h>
#include <Rocket/Core/Variant.h>
#include <Rocket/Core/Dictionary.h>


namespace Rocket {
namespace Core {
namespace Lua {

template<> void ExtraInit<EventParametersProxy>(lua_State* L, int metatable_index)
{
    lua_pushcfunction(L,EventParametersProxy__index);
    lua_setfield(L,metatable_index,"__index");
    lua_pushcfunction(L,EventParametersProxy__pairs);
    lua_setfield(L,metatable_index,"__pairs");
    lua_pushcfunction(L,EventParametersProxy__ipairs);
    lua_setfield(L,metatable_index,"__ipairs");
}

int EventParametersProxy__index(lua_State* L)
{
    /*the table obj and the missing key are currently on the stack(index 1 & 2) as defined by the Lua language*/
    int keytype = lua_type(L,2);
    if(keytype == LUA_TSTRING) //only valid key types
    {
        EventParametersProxy* obj = LuaType<EventParametersProxy>::check(L,1);
        LUACHECKOBJ(obj);
        const char* key = lua_tostring(L,2);
        Variant* param = obj->owner->GetParameters()->Get(key);
        PushVariant(L,param);
        return 1;
    }
    else
        return LuaType<EventParametersProxy>::index(L);
}


//[1] is the object, [2] is the last used key, [3] is the userdata
int EventParametersProxy__pairs(lua_State* L)
{
    EventParametersProxy* obj = LuaType<EventParametersProxy>::check(L,1);
    LUACHECKOBJ(obj);
    int* pindex = (int*)lua_touserdata(L,3);
    if((*pindex) == -1)
        *pindex = 0;
    String key = "";
    Variant* value = NULL;
    if(obj->owner->GetParameters()->Iterate((*pindex),key,value))
    {
        lua_pushstring(L,key.CString());
        PushVariant(L,value);
    }
    else
    {
        lua_pushnil(L);
        lua_pushnil(L);
    }
    return 2;
}

//only index by string
int EventParametersProxy__ipairs(lua_State* L)
{
    lua_pushnil(L);
    lua_pushnil(L);
    return 2;
}

RegType<EventParametersProxy> EventParametersProxyMethods[] =
{
    { NULL, NULL },
};
luaL_reg EventParametersProxyGetters[] =
{
    { NULL, NULL },
};
luaL_reg EventParametersProxySetters[] =
{
    { NULL, NULL },
};

LUACORETYPEDEFINE(EventParametersProxy,false)
}
}
}