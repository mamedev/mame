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
#include "ElementFormControl.h"
#include <Rocket/Controls/ElementFormControl.h>
#include <Rocket/Core/Element.h>
#include <Rocket/../../Source/Core/Lua/Element.h>
#include <Rocket/Core/Lua/Utilities.h>


namespace Rocket {
namespace Controls {
namespace Lua {

//getters
int ElementFormControlGetAttrdisabled(lua_State* L)
{
    ElementFormControl* efc = LuaType<ElementFormControl>::check(L,1);
    LUACHECKOBJ(efc);
    lua_pushboolean(L,efc->IsDisabled());
    return 1;
}

int ElementFormControlGetAttrname(lua_State* L)
{
    ElementFormControl* efc = LuaType<ElementFormControl>::check(L,1);
    LUACHECKOBJ(efc);
    lua_pushstring(L,efc->GetName().CString());
    return 1;
}

int ElementFormControlGetAttrvalue(lua_State* L)
{
    ElementFormControl* efc = LuaType<ElementFormControl>::check(L,1);
    LUACHECKOBJ(efc);
    lua_pushstring(L,efc->GetValue().CString());
    return 1;
}


//setters
int ElementFormControlSetAttrdisabled(lua_State* L)
{
    ElementFormControl* efc = LuaType<ElementFormControl>::check(L,1);
    LUACHECKOBJ(efc);
    efc->SetDisabled(CHECK_BOOL(L,2));
    return 0;
}

int ElementFormControlSetAttrname(lua_State* L)
{
    ElementFormControl* efc = LuaType<ElementFormControl>::check(L,1);
    LUACHECKOBJ(efc);
    const char* name = luaL_checkstring(L,2);
    efc->SetName(name);
    return 0;
}

int ElementFormControlSetAttrvalue(lua_State* L)
{
    ElementFormControl* efc = LuaType<ElementFormControl>::check(L,1);
    LUACHECKOBJ(efc);
    const char* value = luaL_checkstring(L,2);
    efc->SetValue(value);
    return 0;
}


Rocket::Core::Lua::RegType<ElementFormControl> ElementFormControlMethods[] = 
{
    { NULL, NULL },
};

luaL_reg ElementFormControlGetters[] = 
{
    LUAGETTER(ElementFormControl,disabled)
    LUAGETTER(ElementFormControl,name)
    LUAGETTER(ElementFormControl,value)
    { NULL, NULL },
};

luaL_reg ElementFormControlSetters[] = 
{
    LUASETTER(ElementFormControl,disabled)
    LUASETTER(ElementFormControl,name)
    LUASETTER(ElementFormControl,value)
    { NULL, NULL },
};

}
}
}
namespace Rocket {
namespace Core {
namespace Lua {
template<> void ExtraInit<Rocket::Controls::ElementFormControl>(lua_State* L, int metatable_index)
{
    ExtraInit<Element>(L,metatable_index);
    LuaType<Element>::_regfunctions(L,metatable_index,metatable_index-1);
    AddTypeToElementAsTable<Rocket::Controls::ElementFormControl>(L);
}
using Rocket::Controls::ElementFormControl;
LUACONTROLSTYPEDEFINE(ElementFormControl,true)
}
}
}
