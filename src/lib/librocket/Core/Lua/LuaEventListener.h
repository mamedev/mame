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
 
#ifndef ROCKETCORELUALUAEVENTLISTENER_H
#define ROCKETCORELUALUAEVENTLISTENER_H

#include <Rocket/Core/EventListener.h>
#include <Rocket/Core/String.h>
#include <Rocket/Core/Lua/lua.hpp>

namespace Rocket {
namespace Core {
namespace Lua {

class LuaEventListener : public EventListener
{
public:
    //The plan is to wrap the code in an anonymous function so that we can have named parameters to use,
    //rather than putting them in global variables
    LuaEventListener(const String& code, Element* element);

    //This is called from a Lua Element if in element:AddEventListener it passes a function in as the 2nd
    //parameter rather than a string. We don't wrap the function in an anonymous function, so the user
    //should take care to have the proper order. The order is event,element,document.
	//narg is the position on the stack
    LuaEventListener(lua_State* L, int narg, Element* element);

    virtual ~LuaEventListener();

    /// Process the incoming Event
	virtual void ProcessEvent(Event& event);
private:
    //the lua-side function to call when ProcessEvent is called
    int luaFuncRef;
    Element* attached;
    ElementDocument* parent;
    String strFunc; //for debugging purposes
};

}
}
}
#endif
