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
#include "ElementFormControlInput.h"
#include <Rocket/Controls/ElementFormControl.h>
#include "ElementFormControl.h"
#include <Rocket/Core/Lua/Utilities.h>

namespace Rocket {
namespace Controls {
namespace Lua {


//getters
int ElementFormControlInputGetAttrchecked(lua_State* L)
{
    ElementFormControlInput* obj = LuaType<ElementFormControlInput>::check(L,1);
    LUACHECKOBJ(obj);
    lua_pushboolean(L,obj->HasAttribute("checked"));
    return 1;
}

int ElementFormControlInputGetAttrmaxlength(lua_State* L)
{
    ElementFormControlInput* obj = LuaType<ElementFormControlInput>::check(L,1);
    LUACHECKOBJ(obj);
    lua_pushinteger(L,obj->GetAttribute<int>("maxlength",-1));
    return 1;
}

int ElementFormControlInputGetAttrsize(lua_State* L)
{
    ElementFormControlInput* obj = LuaType<ElementFormControlInput>::check(L,1);
    LUACHECKOBJ(obj);
    lua_pushinteger(L,obj->GetAttribute<int>("size",20));
    return 1;
}

int ElementFormControlInputGetAttrmax(lua_State* L)
{
    ElementFormControlInput* obj = LuaType<ElementFormControlInput>::check(L,1);
    LUACHECKOBJ(obj);
    lua_pushinteger(L,obj->GetAttribute<int>("max",100));
    return 1;
}

int ElementFormControlInputGetAttrmin(lua_State* L)
{
    ElementFormControlInput* obj = LuaType<ElementFormControlInput>::check(L,1);
    LUACHECKOBJ(obj);
    lua_pushinteger(L,obj->GetAttribute<int>("min",0));
    return 1;
}

int ElementFormControlInputGetAttrstep(lua_State* L)
{
    ElementFormControlInput* obj = LuaType<ElementFormControlInput>::check(L,1);
    LUACHECKOBJ(obj);
    lua_pushinteger(L,obj->GetAttribute<int>("step",1));
    return 1;
}


//setters
int ElementFormControlInputSetAttrchecked(lua_State* L)
{
    ElementFormControlInput* obj = LuaType<ElementFormControlInput>::check(L,1);
    LUACHECKOBJ(obj);
    bool checked = CHECK_BOOL(L,2);
    if(checked)
        obj->SetAttribute("checked",true);
    else
        obj->RemoveAttribute("checked");
    return 0;
}

int ElementFormControlInputSetAttrmaxlength(lua_State* L)
{
    ElementFormControlInput* obj = LuaType<ElementFormControlInput>::check(L,1);
    LUACHECKOBJ(obj);
    int maxlength = luaL_checkint(L,2);
    obj->SetAttribute("maxlength",maxlength);
    return 0;
}

int ElementFormControlInputSetAttrsize(lua_State* L)
{
    ElementFormControlInput* obj = LuaType<ElementFormControlInput>::check(L,1);
    LUACHECKOBJ(obj);
    int size = luaL_checkint(L,2);
    obj->SetAttribute("size",size);
    return 0;
}

int ElementFormControlInputSetAttrmax(lua_State* L)
{
    ElementFormControlInput* obj = LuaType<ElementFormControlInput>::check(L,1);
    LUACHECKOBJ(obj);
    int max = luaL_checkint(L,2);
    obj->SetAttribute("max",max);
    return 0;
}

int ElementFormControlInputSetAttrmin(lua_State* L)
{
    ElementFormControlInput* obj = LuaType<ElementFormControlInput>::check(L,1);
    LUACHECKOBJ(obj);
    int min = luaL_checkint(L,2);
    obj->SetAttribute("min",min);
    return 0;
}

int ElementFormControlInputSetAttrstep(lua_State* L)
{
    ElementFormControlInput* obj = LuaType<ElementFormControlInput>::check(L,1);
    LUACHECKOBJ(obj);
    int step = luaL_checkint(L,2);
    obj->SetAttribute("step",step);
    return 0;
}


Rocket::Core::Lua::RegType<ElementFormControlInput> ElementFormControlInputMethods[] = 
{
    {NULL,NULL},
};

luaL_reg ElementFormControlInputGetters[] = 
{
    LUAGETTER(ElementFormControlInput,checked)
    LUAGETTER(ElementFormControlInput,maxlength)
    LUAGETTER(ElementFormControlInput,size)
    LUAGETTER(ElementFormControlInput,max)
    LUAGETTER(ElementFormControlInput,min)
    LUAGETTER(ElementFormControlInput,step)
    {NULL,NULL},
};

luaL_reg ElementFormControlInputSetters[] = 
{
    LUASETTER(ElementFormControlInput,checked)
    LUASETTER(ElementFormControlInput,maxlength)
    LUASETTER(ElementFormControlInput,size)
    LUASETTER(ElementFormControlInput,max)
    LUASETTER(ElementFormControlInput,min)
    LUASETTER(ElementFormControlInput,step)
    {NULL,NULL},
};

}
}
}
namespace Rocket {
namespace Core {
namespace Lua {
template<> void ExtraInit<Rocket::Controls::ElementFormControlInput>(lua_State* L, int metatable_index)
{
    ExtraInit<Rocket::Controls::ElementFormControl>(L,metatable_index);
    LuaType<Rocket::Controls::ElementFormControl>::_regfunctions(L,metatable_index,metatable_index-1);
    AddTypeToElementAsTable<Rocket::Controls::ElementFormControlInput>(L);
}
using Rocket::Controls::ElementFormControlInput;
LUACONTROLSTYPEDEFINE(ElementFormControlInput,true)
}
}
}