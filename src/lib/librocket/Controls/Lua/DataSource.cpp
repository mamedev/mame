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
#include "DataSource.h"
#include <Rocket/Core/Log.h>

using Rocket::Core::Log;

namespace Rocket {
namespace Controls {
namespace Lua {
typedef LuaDataSource DataSource;

int DataSourcenew(lua_State* L)
{
    const char* name = luaL_checkstring(L,1);
    LuaDataSource* ds = new LuaDataSource(name);
    LuaType<DataSource>::push(L,ds,true);
    return 1;
}

int DataSourceNotifyRowAdd(lua_State* L, DataSource* obj)
{
    LUACHECKOBJ(obj);
    const char* table_name = luaL_checkstring(L,1);
    int first_row_added = luaL_checkint(L,2);
    int num_rows_added = luaL_checkint(L,3);
    obj->NotifyRowAdd(table_name,first_row_added,num_rows_added);
    return 0;
}

int DataSourceNotifyRowRemove(lua_State* L, DataSource* obj)
{
    LUACHECKOBJ(obj);
    const char* table_name = luaL_checkstring(L,1);
    int first_row_removed = luaL_checkint(L,2);
    int num_rows_removed = luaL_checkint(L,3);
    obj->NotifyRowRemove(table_name,first_row_removed,num_rows_removed);
    return 0;
}

int DataSourceNotifyRowChange(lua_State* L, DataSource* obj)
{
    LUACHECKOBJ(obj);
    const char* table_name = luaL_checkstring(L,1);
    if(lua_gettop(L) < 2)
    {
        obj->NotifyRowChange(table_name);
    }
    else
    {
        int first_row_changed = luaL_checkint(L,2);
        int num_rows_changed = luaL_checkint(L,3);
        obj->NotifyRowChange(table_name,first_row_changed,num_rows_changed);
    }
    return 0;
}

int DataSourceSetAttrGetNumRows(lua_State* L)
{
    DataSource* obj = LuaType<DataSource>::check(L,1);
    LUACHECKOBJ(obj);
    if(lua_type(L,2) == LUA_TFUNCTION)
    {
        lua_pushvalue(L,2); //copy of the function, so it is for sure at the top of the stack
        obj->getNumRowsRef = lua_ref(L,true);
    }
    else
        Log::Message(Log::LT_WARNING, "Lua: Must assign DataSource.GetNumRows as a function, value received was of %s type", lua_typename(L,2));
    return 0;
}

int DataSourceSetAttrGetRow(lua_State* L)
{
    DataSource* obj = LuaType<DataSource>::check(L,1);
    LUACHECKOBJ(obj);
    if(lua_type(L,2) == LUA_TFUNCTION)
    {
        lua_pushvalue(L,2); //copy of the functions, so it is for sure at the top of the stack
        obj->getRowRef = lua_ref(L,true);
    }
    else
        Log::Message(Log::LT_WARNING, "Lua: Must assign DataSource.GetRow as a function, value received was of %s type", lua_typename(L,2));
    return 0;
}


Rocket::Core::Lua::RegType<DataSource> DataSourceMethods[] =
{
    LUAMETHOD(DataSource,NotifyRowAdd)
    LUAMETHOD(DataSource,NotifyRowRemove)
    LUAMETHOD(DataSource,NotifyRowChange)
    { NULL, NULL },
};

luaL_reg DataSourceGetters[] =
{
    { NULL, NULL },
};

luaL_reg DataSourceSetters[] =
{
    LUASETTER(DataSource,GetNumRows)
    LUASETTER(DataSource,GetRow)
    { NULL, NULL },
};



}
}
}
namespace Rocket {
namespace Core {
namespace Lua {
template<> void ExtraInit<Rocket::Controls::Lua::LuaDataSource>(lua_State* L, int metatable_index) 
{ 
    lua_pushcfunction(L,Rocket::Controls::Lua::DataSourcenew);
    lua_setfield(L,metatable_index-1,"new");
    return;
}
using Rocket::Controls::Lua::DataSource;
LUACONTROLSTYPEDEFINE(DataSource,false)
}
}
}
