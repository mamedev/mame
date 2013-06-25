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
#include "ElementForm.h"
#include <Rocket/Core/Element.h>
#include <Rocket/Controls/ElementForm.h>
#include <Rocket/Core/Lua/Utilities.h>

namespace Rocket {
namespace Controls {
namespace Lua {

//method
int ElementFormSubmit(lua_State* L, ElementForm* obj)
{
    int top = lua_gettop(L);
    const char* name = "";
    const char* value = "";
    if(top > 0)
    {
        name = luaL_checkstring(L,1);
        if(top > 1)
            value = luaL_checkstring(L,2);
    }
    obj->Submit(name,value);
    return 0;
}

Rocket::Core::Lua::RegType<ElementForm> ElementFormMethods[] =
{
    LUAMETHOD(ElementForm,Submit)
    { NULL, NULL },
};

luaL_reg ElementFormGetters[] =
{
    { NULL, NULL },
};

luaL_reg ElementFormSetters[] =
{
    { NULL, NULL },
};


}
}
}
namespace Rocket {
namespace Core {
namespace Lua {
template<> void ExtraInit<Rocket::Controls::ElementForm>(lua_State* L, int metatable_index)
{
    //inherit from Element
    ExtraInit<Element>(L,metatable_index);
    LuaType<Element>::_regfunctions(L,metatable_index,metatable_index-1);
    AddTypeToElementAsTable<Rocket::Controls::ElementForm>(L);
}
using Rocket::Controls::ElementForm;
LUACONTROLSTYPEDEFINE(ElementForm,true)
}
}
}
