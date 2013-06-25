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
#include "SelectOptionsProxy.h"
#include <Rocket/Core/Element.h>


namespace Rocket {
namespace Controls {
namespace Lua {


int SelectOptionsProxy__index(lua_State* L)
{
    /*the table obj and the missing key are currently on the stack(index 1 & 2) as defined by the Lua language*/
    int keytype = lua_type(L,2);
    if(keytype == LUA_TNUMBER) //only valid key types
    {
        SelectOptionsProxy* proxy = LuaType<SelectOptionsProxy>::check(L,1);
        LUACHECKOBJ(proxy);
        int index = luaL_checkint(L,2);
        Rocket::Controls::SelectOption* opt = proxy->owner->GetOption(index);
        LUACHECKOBJ(opt);
        lua_newtable(L);
        LuaType<Rocket::Core::Element>::push(L,opt->GetElement(),false);
        lua_setfield(L,-2,"element");
        lua_pushstring(L,opt->GetValue().CString());
        lua_setfield(L,-2,"value");
        return 1;
    }
    else
        return LuaType<SelectOptionsProxy>::index(L);
}

//since there are no string keys, just use __ipairs
int SelectOptionsProxy__pairs(lua_State* L)
{
    return SelectOptionsProxy__ipairs(L);
}

//[1] is the object, [2] is the previous key, [3] is the userdata
int SelectOptionsProxy__ipairs(lua_State* L)
{
    SelectOptionsProxy* proxy = LuaType<SelectOptionsProxy>::check(L,1);
    LUACHECKOBJ(proxy);
    int* pindex = (int*)lua_touserdata(L,3);
    if((*pindex) == -1)
        *pindex = 0;
    SelectOption* opt = NULL;
    while((*pindex) < proxy->owner->GetNumOptions())
    {
        opt = proxy->owner->GetOption((*pindex)++);
        if(opt != NULL) 
            break;
    }
    //we got to the end without finding an option
    if(opt == NULL)
    {
        lua_pushnil(L);
        lua_pushnil(L);
    }
    else //we found an option
    {
        lua_pushinteger(L,(*pindex)-1); //key
        lua_newtable(L); //value
        //fill the value
        LuaType<Rocket::Core::Element>::push(L,opt->GetElement());
        lua_setfield(L,-2,"element");
        lua_pushstring(L,opt->GetValue().CString());
        lua_setfield(L,-2,"value");
    }
    return 2;
}

Rocket::Core::Lua::RegType<SelectOptionsProxy> SelectOptionsProxyMethods[] =
{
    { NULL, NULL },
};

luaL_reg SelectOptionsProxyGetters[] =
{
    { NULL, NULL },
};

luaL_reg SelectOptionsProxySetters[] =
{
    { NULL, NULL },
};


}
}
}
namespace Rocket {
namespace Core {
namespace Lua {
template<> void ExtraInit<Rocket::Controls::Lua::SelectOptionsProxy>(lua_State* L, int metatable_index)
{
    lua_pushcfunction(L,Rocket::Controls::Lua::SelectOptionsProxy__index);
    lua_setfield(L,metatable_index,"__index");
    lua_pushcfunction(L,Rocket::Controls::Lua::SelectOptionsProxy__pairs);
    lua_setfield(L,metatable_index,"__pairs");
    lua_pushcfunction(L,Rocket::Controls::Lua::SelectOptionsProxy__ipairs);
    lua_setfield(L,metatable_index,"__ipairs");
}

using Rocket::Controls::Lua::SelectOptionsProxy;
LUACONTROLSTYPEDEFINE(SelectOptionsProxy,false);
}
}
}