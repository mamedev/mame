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
#include <Rocket/Core/Lua/LuaType.h>
#include <Rocket/Core/Lua/lua.hpp>
#include <Rocket/Core/Event.h>
#include <Rocket/Core/Element.h>
#include <Rocket/Core/Dictionary.h>
#include "EventParametersProxy.h"


namespace Rocket {
namespace Core {
namespace Lua {
template<> void ExtraInit<Event>(lua_State* L, int metatable_index) { return; }

//method
int EventStopPropagation(lua_State* L, Event* obj)
{
    obj->StopPropagation();
    return 0;
}

//getters
int EventGetAttrcurrent_element(lua_State* L)
{
    Event* evt = LuaType<Event>::check(L,1);
    LUACHECKOBJ(evt);
    Element* ele = evt->GetCurrentElement();
    LuaType<Element>::push(L,ele,false);
    return 1;
}

int EventGetAttrtype(lua_State* L)
{
    Event* evt = LuaType<Event>::check(L,1);
    LUACHECKOBJ(evt);
    String type = evt->GetType();
    lua_pushstring(L,type.CString());
    return 1;
}

int EventGetAttrtarget_element(lua_State* L)
{
    Event* evt = LuaType<Event>::check(L,1);
    LUACHECKOBJ(evt);
    Element* target = evt->GetTargetElement();
    LuaType<Element>::push(L,target,false);
    return 1;
}

int EventGetAttrparameters(lua_State* L)
{
    Event* evt = LuaType<Event>::check(L,1);
    LUACHECKOBJ(evt);
    EventParametersProxy* proxy = new EventParametersProxy();
    proxy->owner = evt;
    LuaType<EventParametersProxy>::push(L,proxy,true);
    return 1;
}

RegType<Event> EventMethods[] =
{
    LUAMETHOD(Event,StopPropagation)
    { NULL, NULL },
};

luaL_reg EventGetters[] =
{
    LUAGETTER(Event,current_element)
    LUAGETTER(Event,type)
    LUAGETTER(Event,target_element)
    LUAGETTER(Event,parameters)
    { NULL, NULL },
};

luaL_reg EventSetters[] =
{
    { NULL, NULL },
};

LUACORETYPEDEFINE(Event,true)
}
}
}