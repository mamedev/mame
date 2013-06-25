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
#include "Vector2f.h"
#include <Rocket/Core/Vector2.h>

namespace Rocket {
namespace Core {
namespace Lua {

template<> void ExtraInit<Vector2f>(lua_State* L, int metatable_index)
{
    lua_pushcfunction(L,Vector2fnew);
    lua_setfield(L,metatable_index-1,"new");

    lua_pushcfunction(L,Vector2f__mul);
    lua_setfield(L,metatable_index,"__mul");

    lua_pushcfunction(L,Vector2f__div);
    lua_setfield(L,metatable_index,"__div");

    lua_pushcfunction(L,Vector2f__add);
    lua_setfield(L,metatable_index,"__add");

    lua_pushcfunction(L,Vector2f__sub);
    lua_setfield(L,metatable_index,"__sub");

    lua_pushcfunction(L,Vector2f__eq);
    lua_setfield(L,metatable_index,"__eq");

    //stack is in the same state as it was before it entered this function
    return;
}

int Vector2fnew(lua_State* L)
{
    float x = (float)luaL_checknumber(L,1);
    float y = (float)luaL_checknumber(L,2);

    Vector2f* vect = new Vector2f(x,y);

    LuaType<Vector2f>::push(L,vect,true); //true means it will be deleted when it is garbage collected
    return 1;
}

int Vector2f__mul(lua_State* L)
{
    Vector2f* lhs = LuaType<Vector2f>::check(L,1);
    LUACHECKOBJ(lhs);
    float rhs = (float)luaL_checknumber(L,2);

    Vector2f* res = new Vector2f(0.f,0.f);
    (*res) = (*lhs) * rhs;

    LuaType<Vector2f>::push(L,res,true);
    return 1;
}

int Vector2f__div(lua_State* L)
{
    Vector2f* lhs = LuaType<Vector2f>::check(L,1);
    LUACHECKOBJ(lhs);
    float rhs = (float)luaL_checknumber(L,2);

    Vector2f* res = new Vector2f(0.f,0.f);
    (*res) = (*lhs) / rhs;

    LuaType<Vector2f>::push(L,res,true);
    return 1;
}

int Vector2f__add(lua_State* L)
{
    Vector2f* lhs = LuaType<Vector2f>::check(L,1);
    LUACHECKOBJ(lhs);
    Vector2f* rhs = LuaType<Vector2f>::check(L,2);
    LUACHECKOBJ(rhs);

    Vector2f* res = new Vector2f(0.f,0.f);
    (*res) = (*lhs) + (*rhs);

    LuaType<Vector2f>::push(L,res,true);
    return 1;
}

int Vector2f__sub(lua_State* L)
{
    Vector2f* lhs = LuaType<Vector2f>::check(L,1);
    LUACHECKOBJ(lhs);
    Vector2f* rhs = LuaType<Vector2f>::check(L,2);
    LUACHECKOBJ(rhs);

    Vector2f* res = new Vector2f(0.f,0.f);
    (*res) = (*lhs) - (*rhs);

    LuaType<Vector2f>::push(L,res,true);
    return 1;
}

int Vector2f__eq(lua_State* L)
{
    Vector2f* lhs = LuaType<Vector2f>::check(L,1);
    LUACHECKOBJ(lhs);
    Vector2f* rhs = LuaType<Vector2f>::check(L,2);
    LUACHECKOBJ(rhs);

    lua_pushboolean(L, (*lhs) == (*rhs) ? 1 : 0);
    return 1;
}


int Vector2fDotProduct(lua_State* L, Vector2f* obj)
{
    Vector2f* rhs = LuaType<Vector2f>::check(L,1);
    LUACHECKOBJ(rhs);
    
    float res = obj->DotProduct(*rhs);

    lua_pushnumber(L,res);
    return 1;
}

int Vector2fNormalise(lua_State* L, Vector2f* obj)
{
    Vector2f* res = new Vector2f();
    (*res) = obj->Normalise();

    LuaType<Vector2f>::push(L,res,true);
    return 1;
}

int Vector2fRotate(lua_State* L, Vector2f* obj)
{
    float num = (float)luaL_checknumber(L,1);

    Vector2f* res = new Vector2f();
    (*res) = obj->Rotate(num);

    LuaType<Vector2f>::push(L,res,true);
    return 1;
}

int Vector2fGetAttrx(lua_State*L)
{
    Vector2f* self = LuaType<Vector2f>::check(L,1);
    LUACHECKOBJ(self);

    lua_pushnumber(L,self->x);
    return 1;
}

int Vector2fGetAttry(lua_State*L)
{
    Vector2f* self = LuaType<Vector2f>::check(L,1);
    LUACHECKOBJ(self);

    lua_pushnumber(L,self->y);
    return 1;
}

int Vector2fGetAttrmagnitude(lua_State*L)
{
    Vector2f* self = LuaType<Vector2f>::check(L,1);
    LUACHECKOBJ(self);

    lua_pushnumber(L,self->Magnitude());
    return 1;
}

int Vector2fSetAttrx(lua_State*L)
{
    Vector2f* self = LuaType<Vector2f>::check(L,1);
    LUACHECKOBJ(self);
    float value = (float)luaL_checknumber(L,2);

    self->x = value;
    return 0;
}

int Vector2fSetAttry(lua_State*L)
{
    Vector2f* self = LuaType<Vector2f>::check(L,1);
    LUACHECKOBJ(self);
    float value = (float)luaL_checknumber(L,2);

    self->y = value;
    return 0;
}



RegType<Vector2f> Vector2fMethods[] = 
{
    LUAMETHOD(Vector2f,DotProduct)
    LUAMETHOD(Vector2f,Normalise)
    LUAMETHOD(Vector2f,Rotate)
    { NULL, NULL },
};

luaL_reg Vector2fGetters[]= 
{
    LUAGETTER(Vector2f,x)
    LUAGETTER(Vector2f,y)
    LUAGETTER(Vector2f,magnitude)
    { NULL, NULL },
};

luaL_reg Vector2fSetters[]= 
{
    LUASETTER(Vector2f,x)
    LUASETTER(Vector2f,y)
    { NULL, NULL },
};

LUACORETYPEDEFINE(Vector2f,false)

}
}
}