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
#include "Colourb.h"


namespace Rocket {
namespace Core {
namespace Lua {

template<> void ExtraInit<Colourb>(lua_State* L, int metatable_index) 
{
    lua_pushcfunction(L,Colourbnew);
    lua_setfield(L,metatable_index-1,"new");

    lua_pushcfunction(L,Colourb__eq);
    lua_setfield(L,metatable_index,"__eq");

    lua_pushcfunction(L,Colourb__add);
    lua_setfield(L,metatable_index,"__add");

    lua_pushcfunction(L,Colourb__mul);
    lua_setfield(L,metatable_index,"__mul");

    return;
}
int Colourbnew(lua_State* L)
{
    byte red = (byte)luaL_checkint(L,1);
    byte green = (byte)luaL_checkint(L,2);
    byte blue = (byte)luaL_checkint(L,3);
    byte alpha = (byte)luaL_checkint(L,4);

    Colourb* col = new Colourb(red,green,blue,alpha);

    LuaType<Colourb>::push(L,col,true);
    return 1;
}

int Colourb__eq(lua_State* L)
{
    Colourb* lhs = LuaType<Colourb>::check(L,1);
    LUACHECKOBJ(lhs);
    Colourb* rhs = LuaType<Colourb>::check(L,2);
    LUACHECKOBJ(rhs);

    lua_pushboolean(L, (*lhs) == (*rhs) ? 1 : 0);
    return 1;
}

int Colourb__add(lua_State* L)
{
    Colourb* lhs = LuaType<Colourb>::check(L,1);
    LUACHECKOBJ(lhs);
    Colourb* rhs = LuaType<Colourb>::check(L,2);
    LUACHECKOBJ(rhs);

    Colourb* res = new Colourb((*lhs) + (*rhs));

    LuaType<Colourb>::push(L,res,true);
    return 1;
}

int Colourb__mul(lua_State* L)
{
    Colourb* lhs = LuaType<Colourb>::check(L,1);
    LUACHECKOBJ(lhs);
    float rhs = (float)luaL_checknumber(L,2);

    Colourb* res = new Colourb((*lhs) * rhs);
    
    LuaType<Colourb>::push(L,res,true);
    return 1;
}



//getters
int ColourbGetAttrred(lua_State* L)
{
    Colourb* obj = LuaType<Colourb>::check(L,1);
    LUACHECKOBJ(obj);
    lua_pushinteger(L,obj->red);
    return 1;
}

int ColourbGetAttrgreen(lua_State* L)
{
    Colourb* obj = LuaType<Colourb>::check(L,1);
    LUACHECKOBJ(obj);
    lua_pushinteger(L,obj->green);
    return 1;
}

int ColourbGetAttrblue(lua_State* L)
{
    Colourb* obj = LuaType<Colourb>::check(L,1);
    LUACHECKOBJ(obj);
    lua_pushinteger(L,obj->blue);
    return 1;
}

int ColourbGetAttralpha(lua_State* L)
{
    Colourb* obj = LuaType<Colourb>::check(L,1);
    LUACHECKOBJ(obj);
    lua_pushinteger(L,obj->alpha);
    return 1;
}

int ColourbGetAttrrgba(lua_State* L)
{
    Colourb* obj = LuaType<Colourb>::check(L,1);
    LUACHECKOBJ(obj);
    lua_pushinteger(L,obj->red);
    lua_pushinteger(L,obj->green);
    lua_pushinteger(L,obj->blue);
    lua_pushinteger(L,obj->alpha);
    return 4;
}


//setters
int ColourbSetAttrred(lua_State* L)
{
    Colourb* obj = LuaType<Colourb>::check(L,1);
    LUACHECKOBJ(obj);
    byte red = (byte)luaL_checkinteger(L,2);
    obj->red = red;
    return 0;
}

int ColourbSetAttrgreen(lua_State* L)
{
    Colourb* obj = LuaType<Colourb>::check(L,1);
    LUACHECKOBJ(obj);
    byte green = (byte)luaL_checkinteger(L,2);
    obj->green = green;
    return 0;
}

int ColourbSetAttrblue(lua_State* L)
{
    Colourb* obj = LuaType<Colourb>::check(L,1);
    LUACHECKOBJ(obj);
    byte blue = (byte)luaL_checkinteger(L,2);
    obj->blue = blue;
    return 0;
}

int ColourbSetAttralpha(lua_State* L)
{
    Colourb* obj = LuaType<Colourb>::check(L,1);
    LUACHECKOBJ(obj);
    byte alpha = (byte)luaL_checkinteger(L,2);
    obj->alpha = alpha;
    return 0;
}

int ColourbSetAttrrgba(lua_State* L)
{
    Colourb* obj = NULL;
    int top = lua_gettop(L);
    //each of the items are optional.
    if(top > 0)
    {
        obj = LuaType<Colourb>::check(L,1);
        LUACHECKOBJ(obj);
        if(top > 1)
        {
            if(top > 2)
            {
                if(top > 3)
                    obj->alpha = luaL_checkint(L,4);
                obj->blue = luaL_checkint(L,3);
            }
            obj->green = luaL_checkint(L,2);
        }
        obj->red = luaL_checkint(L,1);
    }
    return 0;
}


RegType<Colourb> ColourbMethods[] =
{
    { NULL, NULL },
};

luaL_reg ColourbGetters[] =
{
    LUAGETTER(Colourb,red)
    LUAGETTER(Colourb,green)
    LUAGETTER(Colourb,blue)
    LUAGETTER(Colourb,alpha)
    LUAGETTER(Colourb,rgba)
    { NULL, NULL },
};

luaL_reg ColourbSetters[] =
{
    LUASETTER(Colourb,red)
    LUASETTER(Colourb,green)
    LUASETTER(Colourb,blue)
    LUASETTER(Colourb,alpha)
    LUASETTER(Colourb,rgba)
    { NULL, NULL },
};

LUACORETYPEDEFINE(Colourb,false)
}
}
}