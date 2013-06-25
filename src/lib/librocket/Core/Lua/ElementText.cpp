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
#include "ElementText.h"
#include "Element.h"
#include <Rocket/Core/Lua/Utilities.h>

namespace Rocket {
namespace Core {
namespace Lua {
template<> void ExtraInit<ElementText>(lua_State* L, int metatable_index)
{
    //inherit from Element
    ExtraInit<Element>(L,metatable_index);
    LuaType<Element>::_regfunctions(L,metatable_index,metatable_index-1);
    AddTypeToElementAsTable<ElementText>(L);
}

int ElementTextGetAttrtext(lua_State* L)
{
    ElementText* obj = LuaType<ElementText>::check(L, 1);
    LUACHECKOBJ(obj);
    String temp;
    lua_pushstring(L,obj->GetText().ToUTF8(temp).CString());
    return 1;
}

int ElementTextSetAttrtext(lua_State* L)
{
    ElementText* obj = LuaType<ElementText>::check(L, 1);
    LUACHECKOBJ(obj);
    const char* text = luaL_checkstring(L,2);
    obj->SetText(text);
    return 0;
}

RegType<ElementText> ElementTextMethods[] =
{
    { NULL, NULL },
};

luaL_reg ElementTextGetters[] =
{
    LUAGETTER(ElementText,text)
    { NULL, NULL },
};

luaL_reg ElementTextSetters[] =
{
    LUASETTER(ElementText,text)
    { NULL, NULL },
};

LUACORETYPEDEFINE(ElementText,true)
}
}
}