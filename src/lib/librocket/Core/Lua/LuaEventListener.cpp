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
#include "LuaEventListener.h"
#include <Rocket/Core/Lua/Interpreter.h>
#include <Rocket/Core/Lua/LuaType.h>
#include <Rocket/Core/Lua/Utilities.h>

namespace Rocket {
namespace Core {
namespace Lua {
typedef Rocket::Core::ElementDocument Document;

LuaEventListener::LuaEventListener(const String& code, Element* element) : EventListener()
{
    //compose function
    String function = "return function (event,element,document) ";
    function.Append(code);
    function.Append(" end");

    //make sure there is an area to save the function
    lua_State* L = Interpreter::GetLuaState();
    int top = lua_gettop(L);
    lua_getglobal(L,"EVENTLISTENERFUNCTIONS");
    if(lua_isnoneornil(L,-1))
    {
        lua_newtable(L);
        lua_setglobal(L,"EVENTLISTENERFUNCTIONS");
        lua_pop(L,1); //pop the unsucessful getglobal
        lua_getglobal(L,"EVENTLISTENERFUNCTIONS");
    }
    int tbl = lua_gettop(L);

    //compile,execute,and save the function
    if(luaL_loadstring(L,function.CString()) != 0)
    {
        Report(L);
        return;
    }
    else
    {
        if(lua_pcall(L,0,1,0) != 0)
        {
            Report(L);
            return;
        }
    }
    luaFuncRef = luaL_ref(L,tbl); //creates a reference to the item at the top of the stack in to the table we just created
    lua_pop(L,1); //pop the EVENTLISTENERFUNCTIONS table

    attached = element;
	if(element)
		parent = element->GetOwnerDocument();
	else
		parent = NULL;
    strFunc = function;
    lua_settop(L,top);
}

//if it is passed in a Lua function
LuaEventListener::LuaEventListener(lua_State* L, int narg, Element* element)
{
    int top = lua_gettop(L);
    lua_getglobal(L,"EVENTLISTENERFUNCTIONS");
	if(lua_isnoneornil(L,-1))
	{
		lua_newtable(L);
		lua_setglobal(L,"EVENTLISTENERFUNCTIONS");
		lua_pop(L,1); //pop the unsucessful getglobal
		lua_getglobal(L,"EVENTLISTENERFUNCTIONS");
	}
	lua_pushvalue(L,narg);
	luaFuncRef = luaL_ref(L,-2); //put the funtion as a ref in to that table
	lua_pop(L,1); //pop the EVENTLISTENERFUNCTIONS table

	attached = element;
	if(element)
		parent = element->GetOwnerDocument();
	else
		parent = NULL;
    lua_settop(L,top);
}

LuaEventListener::~LuaEventListener()
{
    if(attached)
        attached->RemoveReference();
    if(parent)
        parent->RemoveReference();
}

/// Process the incoming Event
void LuaEventListener::ProcessEvent(Event& event)
{
    //not sure if this is the right place to do this, but if the element we are attached to isn't a document, then
    //the 'parent' variable will be NULL, because element->ower_document hasn't been set on the construction. We should
    //correct that
    if(!parent && attached) parent = attached->GetOwnerDocument();
    lua_State* L = Interpreter::GetLuaState();
    int top = lua_gettop(L); 

    //push the arguments
    lua_getglobal(L,"EVENTLISTENERFUNCTIONS");
    lua_rawgeti(L,-1,luaFuncRef);
    LuaType<Event>::push(L,&event,false);
	LuaType<Element>::push(L,attached,false);
    LuaType<Document>::push(L,parent,false);
    
    Interpreter::ExecuteCall(3,0); //call the function at the top of the stack with 3 arguments

    lua_settop(L,top); //balanced stack makes Lua happy
    
}

}
}
}