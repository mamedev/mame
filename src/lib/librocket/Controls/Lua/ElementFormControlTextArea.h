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
 
#ifndef ROCKETCONTROLSLUAELEMENTFORMCONTROLTEXTAREA_H
#define ROCKETCONTROLSLUAELEMENTFORMCONTROLTEXTAREA_H

#include <Rocket/Core/Lua/lua.hpp>
#include <Rocket/Core/Lua/LuaType.h>
#include <Rocket/Controls/ElementFormControlTextArea.h>

using Rocket::Core::Lua::LuaType;
namespace Rocket {
namespace Controls {
namespace Lua {

//getters
int ElementFormControlTextAreaGetAttrcols(lua_State* L);
int ElementFormControlTextAreaGetAttrmaxlength(lua_State* L);
int ElementFormControlTextAreaGetAttrrows(lua_State* L);
int ElementFormControlTextAreaGetAttrwordwrap(lua_State* L);

//setters
int ElementFormControlTextAreaSetAttrcols(lua_State* L);
int ElementFormControlTextAreaSetAttrmaxlength(lua_State* L);
int ElementFormControlTextAreaSetAttrrows(lua_State* L);
int ElementFormControlTextAreaSetAttrwordwrap(lua_State* L);

extern Rocket::Core::Lua::RegType<ElementFormControlTextArea> ElementFormControlTextAreaMethods[];
extern luaL_reg ElementFormControlTextAreaGetters[];
extern luaL_reg ElementFormControlTextAreaSetters[];

}
}
}
namespace Rocket { namespace Core { namespace Lua {
//inherits from ElementFormControl which inherits from Element
template<> void ExtraInit<Rocket::Controls::ElementFormControlTextArea>(lua_State* L, int metatable_index);
LUACONTROLSTYPEDECLARE(Rocket::Controls::ElementFormControlTextArea)
}}}
#endif
