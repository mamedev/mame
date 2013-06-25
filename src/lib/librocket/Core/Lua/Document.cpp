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
#include "Document.h"
#include <Rocket/Core/ElementDocument.h>
#include <Rocket/Core/Context.h>
#include "Element.h"
#include <Rocket/Core/Lua/Utilities.h>

namespace Rocket {
namespace Core {
namespace Lua {

template<> void ExtraInit<Document>(lua_State* L, int metatable_index)
{
    //we will inherit from Element
    ExtraInit<Element>(L,metatable_index);
    LuaType<Element>::_regfunctions(L,metatable_index,metatable_index - 1);
    AddTypeToElementAsTable<Document>(L);
    
    //create the DocumentFocus table
    lua_getglobal(L,"DocumentFocus");
    if(lua_isnoneornil(L,-1))
    {
        lua_pop(L,1); //pop unsucessful getglobal
        lua_newtable(L); //create a table for holding the enum
        lua_pushinteger(L,ElementDocument::NONE);
        lua_setfield(L,-2,"NONE");
        lua_pushinteger(L,ElementDocument::FOCUS);
        lua_setfield(L,-2,"FOCUS");
        lua_pushinteger(L,ElementDocument::MODAL);
        lua_setfield(L,-2,"MODAL");
        lua_setglobal(L,"DocumentFocus");
        
    }
}

//methods
int DocumentPullToFront(lua_State* L, Document* obj)
{
    obj->PullToFront();
    return 0;
}

int DocumentPushToBack(lua_State* L, Document* obj)
{
    obj->PushToBack();
    return 0;
}

int DocumentShow(lua_State* L, Document* obj)
{
    int top = lua_gettop(L);
    if(top == 0)
        obj->Show();
    else
    {
        int flag = luaL_checkinteger(L,1);
        obj->Show(flag);
    }
    return 0;
}

int DocumentHide(lua_State* L, Document* obj)
{
    obj->Hide();
    return 0;
}

int DocumentClose(lua_State* L, Document* obj)
{
    obj->Close();
    return 0;
}

int DocumentCreateElement(lua_State* L, Document* obj)
{
    const char* tag = luaL_checkstring(L,1);
    Element* ele = obj->CreateElement(tag);
    LuaType<Element>::push(L,ele,false);
    return 1;
}

int DocumentCreateTextNode(lua_State* L, Document* obj)
{
    //need ElementText object first
    const char* text = luaL_checkstring(L,1);
    ElementText* et = obj->CreateTextNode(text);
    LuaType<ElementText>::push(L, et, false);
	return 1;
}


//getters
int DocumentGetAttrtitle(lua_State* L)
{
    Document* doc = LuaType<Document>::check(L,1);
    LUACHECKOBJ(doc);
    lua_pushstring(L,doc->GetTitle().CString());
    return 1;
}

int DocumentGetAttrcontext(lua_State* L)
{
    Document* doc = LuaType<Document>::check(L,1);
    LUACHECKOBJ(doc);
    LuaType<Context>::push(L,doc->GetContext(),false);
    return 1;
}


//setters
int DocumentSetAttrtitle(lua_State* L)
{
    Document* doc = LuaType<Document>::check(L,1);
    LUACHECKOBJ(doc);
    const char* title = luaL_checkstring(L,2);
    doc->SetTitle(title);
    return 0;
}


RegType<Document> DocumentMethods[] =
{
    LUAMETHOD(Document,PullToFront)
    LUAMETHOD(Document,PushToBack)
    LUAMETHOD(Document,Show)
    LUAMETHOD(Document,Hide)
    LUAMETHOD(Document,Close)
    LUAMETHOD(Document,CreateElement)
    LUAMETHOD(Document,CreateTextNode)
    { NULL, NULL },
};

luaL_reg DocumentGetters[] =
{
    LUAGETTER(Document,title)
    LUAGETTER(Document,context)
    { NULL, NULL },
};

luaL_reg DocumentSetters[] =
{
    LUASETTER(Document,title)
    { NULL, NULL },
};

LUACORETYPEDEFINE(Document,true)
}
}
}