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
 
#ifndef ROCKETCORELUACONTEXT_H
#define ROCKETCORELUACONTEXT_H

#include <Rocket/Core/Lua/LuaType.h>
#include <Rocket/Core/Lua/lua.hpp>
#include <Rocket/Core/Context.h>


namespace Rocket {
namespace Core {
namespace Lua {
template<> void ExtraInit<Context>(lua_State* L, int metatable_index);

//methods
int ContextAddEventListener(lua_State* L, Context* obj);
int ContextAddMouseCursor(lua_State* L, Context* obj);
int ContextCreateDocument(lua_State* L, Context* obj);
int ContextLoadDocument(lua_State* L, Context* obj);
int ContextLoadMouseCursor(lua_State* L, Context* obj);
int ContextRender(lua_State* L, Context* obj);
int ContextShowMouseCursor(lua_State* L, Context* obj);
int ContextUnloadAllDocuments(lua_State* L, Context* obj);
int ContextUnloadAllMouseCursors(lua_State* L, Context* obj);
int ContextUnloadDocument(lua_State* L, Context* obj);
int ContextUnloadMouseCursor(lua_State* L, Context* obj);
int ContextUpdate(lua_State* L, Context* obj);

//getters
int ContextGetAttrdimensions(lua_State* L);
int ContextGetAttrdocuments(lua_State* L);
int ContextGetAttrfocus_element(lua_State* L);
int ContextGetAttrhover_element(lua_State* L);
int ContextGetAttrname(lua_State* L);
int ContextGetAttrroot_element(lua_State* L);

//setters
int ContextSetAttrdimensions(lua_State* L);


extern RegType<Context> ContextMethods[];
extern luaL_reg ContextGetters[];
extern luaL_reg ContextSetters[];

LUACORETYPEDECLARE(Context)
}
}
}
#endif
