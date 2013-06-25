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
 
#ifndef ROCKETCORELUAVECTOR2F_H
#define ROCKETCORELUAVECTOR2F_H

#include <Rocket/Core/Lua/lua.hpp>
#include <Rocket/Core/Lua/LuaType.h>
#include <Rocket/Core/Types.h>

using Rocket::Core::Vector2f;
namespace Rocket {
namespace Core {
namespace Lua {
template<> void ExtraInit<Vector2f>(lua_State* L, int metatable_index);

int Vector2fnew(lua_State* L);
int Vector2f__mul(lua_State* L);
int Vector2f__div(lua_State* L);
int Vector2f__add(lua_State* L);
int Vector2f__sub(lua_State* L);
int Vector2f__eq(lua_State* L);

int Vector2fDotProduct(lua_State* L, Vector2f* obj);
int Vector2fNormalise(lua_State* L, Vector2f* obj);
int Vector2fRotate(lua_State* L, Vector2f* obj);

int Vector2fGetAttrx(lua_State*L);
int Vector2fGetAttry(lua_State*L);
int Vector2fGetAttrmagnitude(lua_State*L);

int Vector2fSetAttrx(lua_State*L);
int Vector2fSetAttry(lua_State*L);


extern RegType<Vector2f> Vector2fMethods[];
extern luaL_reg Vector2fGetters[];
extern luaL_reg Vector2fSetters[];

LUACORETYPEDECLARE(Vector2f)
}
}
}
#endif
