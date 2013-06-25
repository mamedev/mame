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
#include "LuaDataSource.h"
#include <Rocket/Core/Lua/Interpreter.h>
#include <Rocket/Core/Log.h>
#include <Rocket/Core/String.h>

using Rocket::Core::Lua::Interpreter;
using Rocket::Core::Log;
using Rocket::Core::Lua::LuaType;
namespace Rocket {
namespace Controls {
namespace Lua {

LuaDataSource::LuaDataSource(const Rocket::Core::String& name) : DataSource(name), getRowRef(LUA_NOREF), getNumRowsRef(LUA_NOREF)
{
}

/// Fetches the contents of one row of a table within the data source.
/// @param[out] row The list of values in the table.
/// @param[in] table The name of the table to query.
/// @param[in] row_index The index of the desired row.
/// @param[in] columns The list of desired columns within the row.
void LuaDataSource::GetRow(Rocket::Core::StringList& row, const Rocket::Core::String& table, int row_index, const Rocket::Core::StringList& columns)
{
    if(getRowRef == LUA_NOREF || getRowRef == LUA_REFNIL) return;

    //setup the call
    Interpreter::BeginCall(getRowRef);
    lua_State* L = Interpreter::GetLuaState();
    lua_pushstring(L,table.CString());
    lua_pushinteger(L,row_index);
    lua_newtable(L);
    int index = 0;
    for(Rocket::Core::StringList::const_iterator itr = columns.begin(); itr != columns.end(); ++itr)
    {
        lua_pushstring(L,itr->CString());
        lua_rawseti(L,-2,index++);
    }
    Interpreter::ExecuteCall(3,1); //3 parameters, 1 return. After here, the top of the stack contains the return value

    int res = lua_gettop(L);
    if(lua_type(L,res) == LUA_TTABLE)
    {
        lua_pushnil(L);
        while(lua_next(L,res) != 0)
        {
            //key at -2, value at -1
            row.push_back(luaL_checkstring(L,-1));
            lua_pop(L,1); //pops value, leaves key for next iteration
        }
        lua_pop(L,1); //pop key
    }
    else
        Log::Message(Log::LT_WARNING, "Lua: DataSource.GetRow must return a table, the function it called returned a %s", lua_typename(L,res));

    Interpreter::EndCall(1);
}

/// Fetches the number of rows within one of this data source's tables.
/// @param[in] table The name of the table to query.
/// @return The number of rows within the specified table. Returns -1 in case of an incorrect Lua function.
int LuaDataSource::GetNumRows(const Rocket::Core::String& table)
{
    if(getNumRowsRef == LUA_NOREF || getNumRowsRef == LUA_REFNIL) return -1;

    lua_State* L = Interpreter::GetLuaState();
    Interpreter::BeginCall(getNumRowsRef);
    lua_pushstring(L,table.CString());
    Interpreter::ExecuteCall(1,1); //1 parameter, 1 return. After this, the top of the stack contains the return value

    int res = lua_gettop(L);
    if(lua_type(L,res) == LUA_TNUMBER)
    {
        return luaL_checkint(L,res);
    }
    else
    {
        Log::Message(Log::LT_WARNING, "Lua: DataSource.GetNumRows must return an integer, the function it called returned a %s", lua_typename(L,res));
        return -1;
    }

}

}
}
}
