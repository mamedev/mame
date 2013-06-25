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
#include "Vector2i.h"


namespace Rocket {
namespace Core {
namespace Lua {
template<> void ExtraInit<Vector2i>(lua_State* L, int metatable_index)
{
    lua_pushcfunction(L,Vector2inew);
    lua_setfield(L,metatable_index-1,"new");

    lua_pushcfunction(L,Vector2i__mul);
    lua_setfield(L,metatable_index,"__mul");

    lua_pushcfunction(L,Vector2i__div);
    lua_setfield(L,metatable_index,"__div");

    lua_pushcfunction(L,Vector2i__add);
    lua_setfield(L,metatable_index,"__add");

    lua_pushcfunction(L,Vector2i__sub);
    lua_setfield(L,metatable_index,"__sub");

    lua_pushcfunction(L,Vector2i__eq);
    lua_setfield(L,metatable_index,"__eq");

    //stack is in the same state as it was before it entered this function
    return;
}

int Vector2inew(lua_State* L)
{
    int x = luaL_checkint(L,1);
    int y = luaL_checkint(L,2);

    Vector2i* vect = new Vector2i(x,y);

    LuaType<Vector2i>::push(L,vect,true); //true means it will be deleted when it is garbage collected
    return 1;
}

int Vector2i__mul(lua_State* L)
{
    Vector2i* lhs = LuaType<Vector2i>::check(L,1);
    LUACHECKOBJ(lhs);
    int rhs = luaL_checkint(L,2);

    Vector2i* res = new Vector2i(0,0);
    (*res) = (*lhs) * rhs;

    LuaType<Vector2i>::push(L,res,true);
    return 1;
}

int Vector2i__div(lua_State* L)
{
    Vector2i* lhs = LuaType<Vector2i>::check(L,1);
    LUACHECKOBJ(lhs);
    int rhs = luaL_checkint(L,2);

    Vector2i* res = new Vector2i(0,0);
    (*res) = (*lhs) / rhs;

    LuaType<Vector2i>::push(L,res,true);
    return 1;
}

int Vector2i__add(lua_State* L)
{
    Vector2i* lhs = LuaType<Vector2i>::check(L,1);
    LUACHECKOBJ(lhs);
    Vector2i* rhs = LuaType<Vector2i>::check(L,2);
    LUACHECKOBJ(rhs);

    Vector2i* res = new Vector2i(0,0);
    (*res) = (*lhs) + (*rhs);

    LuaType<Vector2i>::push(L,res,true);
    return 1;
}

int Vector2i__sub(lua_State* L)
{
    Vector2i* lhs = LuaType<Vector2i>::check(L,1);
    LUACHECKOBJ(lhs);
    Vector2i* rhs = LuaType<Vector2i>::check(L,2);
    LUACHECKOBJ(rhs);

    Vector2i* res = new Vector2i(0,0);
    (*res) = (*lhs) - (*rhs);

    LuaType<Vector2i>::push(L,res,true);
    return 1;
}

int Vector2i__eq(lua_State* L)
{
    Vector2i* lhs = LuaType<Vector2i>::check(L,1);
    LUACHECKOBJ(lhs);
    Vector2i* rhs = LuaType<Vector2i>::check(L,2);
    LUACHECKOBJ(rhs);

    lua_pushboolean(L, (*lhs) == (*rhs) ? 1 : 0);
    return 1;
}

int Vector2iGetAttrx(lua_State*L)
{
    Vector2i* self = LuaType<Vector2i>::check(L,1);
    LUACHECKOBJ(self);

    lua_pushinteger(L,self->x);
    return 1;
}

int Vector2iGetAttry(lua_State*L)
{
    Vector2i* self = LuaType<Vector2i>::check(L,1);
    LUACHECKOBJ(self);

    lua_pushinteger(L,self->y);
    return 1;
}

int Vector2iGetAttrmagnitude(lua_State*L)
{
    Vector2i* self = LuaType<Vector2i>::check(L,1);
    LUACHECKOBJ(self);

    lua_pushnumber(L,self->Magnitude());
    return 1;
}

int Vector2iSetAttrx(lua_State*L)
{
    Vector2i* self = LuaType<Vector2i>::check(L,1);
    LUACHECKOBJ(self);
    int value = luaL_checkint(L,2);

    self->x = value;
    return 0;
}

int Vector2iSetAttry(lua_State*L)
{
    Vector2i* self = LuaType<Vector2i>::check(L,1);
    LUACHECKOBJ(self);
    int value = luaL_checkint(L,2);

    self->y = value;
    return 0;
}



RegType<Vector2i> Vector2iMethods[] = 
{
    { NULL, NULL },
};

luaL_reg Vector2iGetters[]= 
{
    LUAGETTER(Vector2i,x)
    LUAGETTER(Vector2i,y)
    LUAGETTER(Vector2i,magnitude)
    { NULL, NULL },
};

luaL_reg Vector2iSetters[]= 
{
    LUASETTER(Vector2i,x)
    LUASETTER(Vector2i,y)
    { NULL, NULL },
};

LUACORETYPEDEFINE(Vector2i,false)

}
}
}