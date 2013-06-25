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
#include "ElementDataGrid.h"
#include <Rocket/Core/Element.h>
#include <Rocket/Controls/ElementDataGridRow.h>
#include <Rocket/Core/Lua/Utilities.h>


namespace Rocket {
namespace Controls {
namespace Lua {


//methods
int ElementDataGridAddColumn(lua_State* L, ElementDataGrid* obj)
{
    LUACHECKOBJ(obj);
    const char* fields = luaL_checkstring(L,1);
    const char* formatter = luaL_checkstring(L,2);
    float width = (float)luaL_checknumber(L,3);
    const char* rml = luaL_checkstring(L,4);

    obj->AddColumn(fields,formatter,width,rml);
    return 0;
}

int ElementDataGridSetDataSource(lua_State* L, ElementDataGrid* obj)
{
    LUACHECKOBJ(obj);
    const char* source = luaL_checkstring(L,1);
    
    obj->SetDataSource(source);
    return 0;
}


//getter
int ElementDataGridGetAttrrows(lua_State* L)
{
    ElementDataGrid* obj = LuaType<ElementDataGrid>::check(L,1);
    LUACHECKOBJ(obj);

    lua_newtable(L);
    int tbl = lua_gettop(L);
    int numrows = obj->GetNumRows();
    ElementDataGridRow* row;
    for(int i = 0; i < numrows; i++)
    {
        row = obj->GetRow(i);
        LuaType<ElementDataGridRow>::push(L,row,false);
        lua_rawseti(L,tbl,i);
    }
    return 1;
}



Rocket::Core::Lua::RegType<ElementDataGrid> ElementDataGridMethods[] =
{
    LUAMETHOD(ElementDataGrid,AddColumn)
    LUAMETHOD(ElementDataGrid,SetDataSource)
    { NULL, NULL },
};

luaL_reg ElementDataGridGetters[] =
{
    LUAGETTER(ElementDataGrid,rows)
    { NULL, NULL },
};

luaL_reg ElementDataGridSetters[] =
{
    { NULL, NULL },
};


}
}
}
namespace Rocket {
namespace Core {
namespace Lua {
template<> void ExtraInit<Rocket::Controls::ElementDataGrid>(lua_State* L, int metatable_index)
{
    ExtraInit<Element>(L,metatable_index);
    LuaType<Element>::_regfunctions(L,metatable_index,metatable_index-1);
    AddTypeToElementAsTable<Rocket::Controls::ElementDataGrid>(L);
}
using Rocket::Controls::ElementDataGrid;
LUACONTROLSTYPEDEFINE(ElementDataGrid,true)
}
}
}
