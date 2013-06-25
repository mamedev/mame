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
#include "LuaElementInstancer.h"
#include <Rocket/Core/Lua/LuaType.h>
#include <Rocket/Core/Lua/Interpreter.h>
#include <Rocket/Core/Log.h>

namespace Rocket {
namespace Core {
namespace Lua {
//This will be called from Rocket::Core::Lua::ElementInstancernew
LuaElementInstancer::LuaElementInstancer(lua_State* L) : ElementInstancer(), ref_InstanceElement(LUA_NOREF)
{
    if(lua_type(L,1) != LUA_TFUNCTION && !lua_isnoneornil(L,1))
    {
        Log::Message(Log::LT_ERROR, "The argument to ElementInstancer.new has to be a function or nil. You passed in a %s.", luaL_typename(L,1));
        return;
    }
    PushFunctionsTable(L); //top of the table is now ELEMENTINSTANCERFUNCTIONS table
    lua_pushvalue(L,1); //copy of the function
    ref_InstanceElement = luaL_ref(L,-2); 
    lua_pop(L,1); //pop the ELEMENTINSTANCERFUNCTIONS table
}

Element* LuaElementInstancer::InstanceElement(Element* ROCKET_UNUSED(parent), const String& tag, const XMLAttributes& ROCKET_UNUSED(attributes))
{
    lua_State* L = Interpreter::GetLuaState();
    int top = lua_gettop(L);
    Element* ret = NULL;
    if(ref_InstanceElement != LUA_REFNIL && ref_InstanceElement != LUA_NOREF)
    {
        PushFunctionsTable(L);
        lua_rawgeti(L,-1,ref_InstanceElement); //push the function
        lua_pushstring(L,tag.CString()); //push the tag
        Interpreter::ExecuteCall(1,1); //we pass in a string, and we want to get an Element back
        ret = LuaType<Element>::check(L,-1);
    }
    else
    {
        Log::Message(Log::LT_WARNING, "Attempt to call the function for ElementInstancer.InstanceElement, the function does not exist.");
    }
    lua_settop(L,top);
    return ret;
}

void LuaElementInstancer::ReleaseElement(Element* element)
{
    delete element;
}

void LuaElementInstancer::Release()
{
    delete this;
}

void LuaElementInstancer::PushFunctionsTable(lua_State* L)
{
    //make sure there is an area to save the function
    lua_getglobal(L,"ELEMENTINSTANCERFUNCTIONS");
    if(lua_isnoneornil(L,-1))
    {
        lua_newtable(L);
        lua_setglobal(L,"ELEMENTINSTANCERFUNCTIONS");
        lua_pop(L,1); //pop the unsucessful getglobal
        lua_getglobal(L,"ELEMENTINSTANCERFUNCTIONS");
    }
}

}
}
}