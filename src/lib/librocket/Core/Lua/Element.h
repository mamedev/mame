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
 
#ifndef ROCKETCORELUAELEMENT_H
#define ROCKETCORELUAELEMENT_H

#include <Rocket/Core/Lua/LuaType.h>
#include <Rocket/Core/Lua/lua.hpp>
#include <Rocket/Core/Element.h>

namespace Rocket {
namespace Core {
namespace Lua {
template<> ROCKETLUA_API void ExtraInit<Element>(lua_State* L, int metatable_index);

int Elementnew(lua_State* L);
//methods
int ElementAddEventListener(lua_State* L, Element* obj);
int ElementAppendChild(lua_State* L, Element* obj);
int ElementBlur(lua_State* L, Element* obj);
int ElementClick(lua_State* L, Element* obj);
int ElementDispatchEvent(lua_State* L, Element* obj);
int ElementFocus(lua_State* L, Element* obj);
int ElementGetAttribute(lua_State* L, Element* obj);
int ElementGetElementById(lua_State* L, Element* obj);
int ElementGetElementsByTagName(lua_State* L, Element* obj);
int ElementHasAttribute(lua_State* L, Element* obj);
int ElementHasChildNodes(lua_State* L, Element* obj);
int ElementInsertBefore(lua_State* L, Element* obj);
int ElementIsClassSet(lua_State* L, Element* obj);
int ElementRemoveAttribute(lua_State* L, Element* obj);
int ElementRemoveChild(lua_State* L, Element* obj);
int ElementReplaceChild(lua_State* L, Element* obj);
int ElementScrollIntoView(lua_State* L, Element* obj);
int ElementSetAttribute(lua_State* L, Element* obj);
int ElementSetClass(lua_State* L, Element* obj);

//getters
int ElementGetAttrattributes(lua_State* L);
int ElementGetAttrchild_nodes(lua_State* L);
int ElementGetAttrclass_name(lua_State* L);
int ElementGetAttrclient_left(lua_State* L);
int ElementGetAttrclient_height(lua_State* L);
int ElementGetAttrclient_top(lua_State* L);
int ElementGetAttrclient_width(lua_State* L);
int ElementGetAttrfirst_child(lua_State* L);
int ElementGetAttrid(lua_State* L);
int ElementGetAttrinner_rml(lua_State* L);
int ElementGetAttrlast_child(lua_State* L);
int ElementGetAttrnext_sibling(lua_State* L);
int ElementGetAttroffset_height(lua_State* L);
int ElementGetAttroffset_left(lua_State* L);
int ElementGetAttroffset_parent(lua_State* L);
int ElementGetAttroffset_top(lua_State* L);
int ElementGetAttroffset_width(lua_State* L);
int ElementGetAttrowner_document(lua_State* L);
int ElementGetAttrparent_node(lua_State* L);
int ElementGetAttrprevious_sibling(lua_State* L);
int ElementGetAttrscroll_height(lua_State* L);
int ElementGetAttrscroll_left(lua_State* L);
int ElementGetAttrscroll_top(lua_State* L);
int ElementGetAttrscroll_width(lua_State* L);
int ElementGetAttrstyle(lua_State* L);
int ElementGetAttrtag_name(lua_State* L);

//setters
int ElementSetAttrclass_name(lua_State* L);
int ElementSetAttrid(lua_State* L);
int ElementSetAttrinner_rml(lua_State* L);
int ElementSetAttrscroll_left(lua_State* L);
int ElementSetAttrscroll_top(lua_State* L);



extern RegType<Element> ElementMethods[];
extern luaL_reg ElementGetters[];
extern luaL_reg ElementSetters[];

LUACORETYPEDECLARE(Element)
}
}
}
#endif
