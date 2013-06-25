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
 
#ifndef ROCKETCORELUAUTILITIES_H
#define ROCKETCORELUAUTILITIES_H
/*
    This file is for free-floating functions that are used across more than one file.
*/
#include <Rocket/Core/Lua/Header.h>
#include <Rocket/Core/Lua/lua.hpp>
#include <Rocket/Core/Lua/LuaType.h>
#include <Rocket/Core/Variant.h>

namespace Rocket {
namespace Core {
namespace Lua {

/** casts the variant to its specific type before pushing it to the stack 
@relates Rocket::Core::Lua::LuaType */
void ROCKETLUA_API PushVariant(lua_State* L, Variant* var);

/** If there are errors on the top of the stack, this will print those out to the log.
@param L A Lua state, and if not passed in, will use the Interpreter's state
@param place A string that will be printed to the log right before the error message, seperated by a space. Set
this when you would get no information about where the error happens.
@relates Rocket::Core::Lua::Interpreter   */
void ROCKETLUA_API Report(lua_State* L = NULL, const Rocket::Core::String& place = "");

//Helper function, so that the types don't have to define individual functions themselves
// to fill the Elements.As table
template<typename ToType>
int CastFromElementTo(lua_State* L)
{
    Element* ele = LuaType<Element>::check(L,1);
    LUACHECKOBJ(ele);
    LuaType<ToType>::push(L,(ToType*)ele,false);
    return 1;
}

//Adds to the Element.As table the name of the type, and the function to use to cast
template<typename T>
void AddTypeToElementAsTable(lua_State* L)
{
    int top = lua_gettop(L);
    lua_getglobal(L,"Element");
    lua_getfield(L,-1,"As");
    if(!lua_isnoneornil(L,-1))
    {
        lua_pushcfunction(L,CastFromElementTo<T>);
        lua_setfield(L,-2,GetTClassName<T>());
    }
    lua_settop(L,top); //pop "As" and "Element"
}
}
}
}


#endif
