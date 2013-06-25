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
#include "ElementFormControlTextArea.h"
#include <Rocket/Controls/ElementFormControl.h>
#include "ElementFormControl.h"
#include <Rocket/Core/Lua/Utilities.h>

namespace Rocket {
namespace Controls {
namespace Lua {

//getters
int ElementFormControlTextAreaGetAttrcols(lua_State* L)
{
    ElementFormControlTextArea* obj = LuaType<ElementFormControlTextArea>::check(L,1);
    LUACHECKOBJ(obj);
    lua_pushinteger(L,obj->GetNumColumns());
    return 1;
}

int ElementFormControlTextAreaGetAttrmaxlength(lua_State* L)
{
    ElementFormControlTextArea* obj = LuaType<ElementFormControlTextArea>::check(L,1);
    LUACHECKOBJ(obj);
    lua_pushinteger(L,obj->GetMaxLength());
    return 1;
}

int ElementFormControlTextAreaGetAttrrows(lua_State* L)
{
    ElementFormControlTextArea* obj = LuaType<ElementFormControlTextArea>::check(L,1);
    LUACHECKOBJ(obj);
    lua_pushinteger(L,obj->GetNumRows());
    return 1;
}

int ElementFormControlTextAreaGetAttrwordwrap(lua_State* L)
{
    ElementFormControlTextArea* obj = LuaType<ElementFormControlTextArea>::check(L,1);
    LUACHECKOBJ(obj);
    lua_pushboolean(L,obj->GetWordWrap());
    return 1;
}


//setters
int ElementFormControlTextAreaSetAttrcols(lua_State* L)
{
    ElementFormControlTextArea* obj = LuaType<ElementFormControlTextArea>::check(L,1);
    LUACHECKOBJ(obj);
    int cols = luaL_checkint(L,2);
    obj->SetNumColumns(cols);
    return 0;
}

int ElementFormControlTextAreaSetAttrmaxlength(lua_State* L)
{
    ElementFormControlTextArea* obj = LuaType<ElementFormControlTextArea>::check(L,1);
    LUACHECKOBJ(obj);
    int ml = luaL_checkint(L,2);
    obj->SetMaxLength(ml);
    return 0;
}

int ElementFormControlTextAreaSetAttrrows(lua_State* L)
{
    ElementFormControlTextArea* obj = LuaType<ElementFormControlTextArea>::check(L,1);
    LUACHECKOBJ(obj);
    int rows = luaL_checkint(L,2);
    obj->SetNumRows(rows);
    return 0;
}

int ElementFormControlTextAreaSetAttrwordwrap(lua_State* L)
{
    ElementFormControlTextArea* obj = LuaType<ElementFormControlTextArea>::check(L,1);
    LUACHECKOBJ(obj);
    bool ww = CHECK_BOOL(L,2);
    obj->SetWordWrap(ww);
    return 0;
}


Rocket::Core::Lua::RegType<ElementFormControlTextArea> ElementFormControlTextAreaMethods[] =
{
    { NULL, NULL },
};

luaL_reg ElementFormControlTextAreaGetters[] =
{
    LUAGETTER(ElementFormControlTextArea,cols)
    LUAGETTER(ElementFormControlTextArea,maxlength)
    LUAGETTER(ElementFormControlTextArea,rows)
    LUAGETTER(ElementFormControlTextArea,wordwrap)
    { NULL, NULL },
};

luaL_reg ElementFormControlTextAreaSetters[] =
{
    LUASETTER(ElementFormControlTextArea,cols)
    LUASETTER(ElementFormControlTextArea,maxlength)
    LUASETTER(ElementFormControlTextArea,rows)
    LUASETTER(ElementFormControlTextArea,wordwrap)
    { NULL, NULL },
};

}
}
}
namespace Rocket {
namespace Core {
namespace Lua {
template<> void ExtraInit<Rocket::Controls::ElementFormControlTextArea>(lua_State* L, int metatable_index)
{
    ExtraInit<Rocket::Controls::ElementFormControl>(L,metatable_index);
    LuaType<Rocket::Controls::ElementFormControl>::_regfunctions(L,metatable_index,metatable_index-1);
    AddTypeToElementAsTable<Rocket::Controls::ElementFormControlTextArea>(L);
}

using Rocket::Controls::ElementFormControlTextArea;
LUACONTROLSTYPEDEFINE(ElementFormControlTextArea,true)
}
}
}