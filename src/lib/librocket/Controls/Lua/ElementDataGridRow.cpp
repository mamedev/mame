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
#include "ElementDataGridRow.h"
#include <Rocket/Controls/ElementDataGrid.h>
#include <Rocket/Core/Lua/Utilities.h>


namespace Rocket {
namespace Controls {
namespace Lua {


//getters
int ElementDataGridRowGetAttrrow_expanded(lua_State* L)
{
    ElementDataGridRow* obj = LuaType<ElementDataGridRow>::check(L,1);
    LUACHECKOBJ(obj);
    lua_pushboolean(L,obj->IsRowExpanded());
    return 1;
}

int ElementDataGridRowGetAttrparent_relative_index(lua_State* L)
{
    ElementDataGridRow* obj = LuaType<ElementDataGridRow>::check(L,1);
    LUACHECKOBJ(obj);
    lua_pushinteger(L,obj->GetParentRelativeIndex());
    return 1;
}

int ElementDataGridRowGetAttrtable_relative_index(lua_State* L)
{
    ElementDataGridRow* obj = LuaType<ElementDataGridRow>::check(L,1);
    LUACHECKOBJ(obj);
    lua_pushinteger(L,obj->GetTableRelativeIndex());
    return 1;
}

int ElementDataGridRowGetAttrparent_row(lua_State* L)
{
    ElementDataGridRow* obj = LuaType<ElementDataGridRow>::check(L,1);
    LUACHECKOBJ(obj);
    LuaType<ElementDataGridRow>::push(L,obj->GetParentRow(),false);
    return 1;
}

int ElementDataGridRowGetAttrparent_grid(lua_State* L)
{
    ElementDataGridRow* obj = LuaType<ElementDataGridRow>::check(L,1);
    LUACHECKOBJ(obj);
    LuaType<ElementDataGrid>::push(L,obj->GetParentGrid(),false);
    return 1;
}


//setter
int ElementDataGridRowSetAttrrow_expanded(lua_State* L)
{
    ElementDataGridRow* obj = LuaType<ElementDataGridRow>::check(L,1);
    LUACHECKOBJ(obj);
    bool expanded = CHECK_BOOL(L,2);
    if(expanded)
        obj->ExpandRow();
    else
        obj->CollapseRow();
    return 0;
}



Rocket::Core::Lua::RegType<ElementDataGridRow> ElementDataGridRowMethods[] =
{
    { NULL, NULL },
};

luaL_reg ElementDataGridRowGetters[] =
{
    LUAGETTER(ElementDataGridRow,row_expanded)
    LUAGETTER(ElementDataGridRow,parent_relative_index)
    LUAGETTER(ElementDataGridRow,table_relative_index)
    LUAGETTER(ElementDataGridRow,parent_row)
    LUAGETTER(ElementDataGridRow,parent_grid)
    { NULL, NULL },
};

luaL_reg ElementDataGridRowSetters[] =
{
    LUASETTER(ElementDataGridRow,row_expanded)
    { NULL, NULL },
};

}
}
}
namespace Rocket {
namespace Core {
namespace Lua {
template<> void ExtraInit<Rocket::Controls::ElementDataGridRow>(lua_State* L, int metatable_index)
{
    ExtraInit<Element>(L,metatable_index);
    LuaType<Element>::_regfunctions(L,metatable_index,metatable_index-1);
    AddTypeToElementAsTable<Rocket::Controls::ElementDataGridRow>(L);
}
using Rocket::Controls::ElementDataGridRow;
LUACONTROLSTYPEDEFINE(ElementDataGridRow,true)
}
}
}