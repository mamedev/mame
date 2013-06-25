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
 
#ifndef ROCKETCORELUACOLOURB_H
#define ROCKETCORELUACOLOURB_H

#include <Rocket/Core/Lua/LuaType.h>
#include <Rocket/Core/Lua/lua.hpp>
#include <Rocket/Core/Types.h>

using Rocket::Core::Colourb;
namespace Rocket {
namespace Core {
namespace Lua {
template<> void ExtraInit<Colourb>(lua_State* L, int metatable_index);
int Colourbnew(lua_State* L);
int Colourb__eq(lua_State* L);
int Colourb__add(lua_State* L);
int Colourb__mul(lua_State* L);


//getters
int ColourbGetAttrred(lua_State* L);
int ColourbGetAttrgreen(lua_State* L);
int ColourbGetAttrblue(lua_State* L);
int ColourbGetAttralpha(lua_State* L);
int ColourbGetAttrrgba(lua_State* L);

//setters
int ColourbSetAttrred(lua_State* L);
int ColourbSetAttrgreen(lua_State* L);
int ColourbSetAttrblue(lua_State* L);
int ColourbSetAttralpha(lua_State* L);
int ColourbSetAttrrgba(lua_State* L);

extern RegType<Colourb> ColourbMethods[];
extern luaL_reg ColourbGetters[];
extern luaL_reg ColourbSetters[];

LUACORETYPEDECLARE(Colourb)
}
}
}
#endif