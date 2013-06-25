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
#include "Context.h"
#include <Rocket/Core/Context.h>
#include <Rocket/Core/ElementDocument.h>
#include <Rocket/Core/Factory.h>
#include "LuaEventListener.h"
#include "ContextDocumentsProxy.h"

namespace Rocket {
namespace Core {
namespace Lua {
typedef Rocket::Core::ElementDocument Document;
template<> void ExtraInit<Context>(lua_State* L, int metatable_index) { return; }

//methods
int ContextAddEventListener(lua_State* L, Context* obj)
{
   //need to make an EventListener for Lua before I can do anything else
	const char* evt = luaL_checkstring(L,1); //event
	Element* element = NULL;
	bool capturephase = false;
	//get the rest of the stuff needed to construct the listener
	if(lua_gettop(L) > 2)
	{
		if(!lua_isnoneornil(L,3))
			element = LuaType<Element>::check(L,3);
		if(!lua_isnoneornil(L,4))
			capturephase = CHECK_BOOL(L,4);

	}
	int type = lua_type(L,2);
	if(type == LUA_TFUNCTION)
	{
		if(element)
			element->AddEventListener(evt, new LuaEventListener(L,2,element), capturephase);
		else
			obj->AddEventListener(evt, new LuaEventListener(L,2,NULL), capturephase);
	}
	else if(type == LUA_TSTRING)
	{
		if(element)
			element->AddEventListener(evt, new LuaEventListener(luaL_checkstring(L,2),element), capturephase);
		else
			obj->AddEventListener(evt, new LuaEventListener(luaL_checkstring(L,2),NULL), capturephase);
	}
	else
	{
		Log::Message(Log::LT_WARNING, "Lua Context:AddEventLisener's 2nd argument can only be a Lua function or a string, you passed in a %s", lua_typename(L,type));
	}
    return 0;
}

int ContextAddMouseCursor(lua_State* L, Context* obj)
{
    Document* cursor_doc = LuaType<Document>::check(L,1);
    obj->AddMouseCursor(cursor_doc);
    return 0;   
}

int ContextCreateDocument(lua_State* L, Context* obj)
{
    const char* tag;
    if(lua_gettop(L) < 1)
        tag = "body";
    else
        tag = luaL_checkstring(L,1);
    Document* doc = obj->CreateDocument(tag);
    LuaType<Document>::push(L,doc,true);
    return 1;
}

int ContextLoadDocument(lua_State* L, Context* obj)
{
    const char* path = luaL_checkstring(L,1);
    Document* doc = obj->LoadDocument(path);
    LuaType<Document>::push(L,doc,false);
	doc->RemoveReference();
    return 1;
}

int ContextLoadMouseCursor(lua_State* L, Context* obj)
{
    const char* path = luaL_checkstring(L,1);
    Document* doc = obj->LoadMouseCursor(path);
    LuaType<Document>::push(L,doc);
    return 1;
}

int ContextRender(lua_State* L, Context* obj)
{
    lua_pushboolean(L,obj->Render());
    return 1;
}

int ContextShowMouseCursor(lua_State* L, Context* obj)
{
    bool show = CHECK_BOOL(L,1);
    obj->ShowMouseCursor(show);
    return 0;
}

int ContextUnloadAllDocuments(lua_State* L, Context* obj)
{
    obj->UnloadAllDocuments();
    return 0;
}

int ContextUnloadAllMouseCursors(lua_State* L, Context* obj)
{
    obj->UnloadAllMouseCursors();
    return 0;
}

int ContextUnloadDocument(lua_State* L, Context* obj)
{
    Document* doc = LuaType<Document>::check(L,1);
    obj->UnloadDocument(doc);
    return 0;
}

int ContextUnloadMouseCursor(lua_State* L, Context* obj)
{
    const char* name = luaL_checkstring(L,1);
    obj->UnloadMouseCursor(name);
    return 0;
}

int ContextUpdate(lua_State* L, Context* obj)
{
    lua_pushboolean(L,obj->Update());
    return 1;
}


//getters
int ContextGetAttrdimensions(lua_State* L)
{
    Context* cont = LuaType<Context>::check(L,1);
    const Vector2i* dim = &cont->GetDimensions();
    //const_cast-ing so that the user can do dimensions.x = 3 and it will actually change the dimensions
    //of the context
    LuaType<Vector2i>::push(L,const_cast<Vector2i*>(dim));
    return 1;
}

//returns a table of everything
int ContextGetAttrdocuments(lua_State* L)
{
    Context* cont = LuaType<Context>::check(L,1);
    LUACHECKOBJ(cont);
    ContextDocumentsProxy* cdp = new ContextDocumentsProxy();
    cdp->owner = cont;
    LuaType<ContextDocumentsProxy>::push(L,cdp,true); //does get garbage collected (deleted)
    return 1;
}

int ContextGetAttrfocus_element(lua_State* L)
{
    Context* cont = LuaType<Context>::check(L,1);
    LUACHECKOBJ(cont);
    LuaType<Element>::push(L,cont->GetFocusElement());
    return 1;
}

int ContextGetAttrhover_element(lua_State* L)
{
    Context* cont = LuaType<Context>::check(L,1);
    LUACHECKOBJ(cont);
    LuaType<Element>::push(L,cont->GetHoverElement());
    return 1;
}

int ContextGetAttrname(lua_State* L)
{
    Context* cont = LuaType<Context>::check(L,1);
    LUACHECKOBJ(cont);
    lua_pushstring(L,cont->GetName().CString());
    return 1;
}

int ContextGetAttrroot_element(lua_State* L)
{
    Context* cont = LuaType<Context>::check(L,1);
    LUACHECKOBJ(cont);
    LuaType<Element>::push(L,cont->GetRootElement());
    return 1;
}


//setters
int ContextSetAttrdimensions(lua_State* L)
{
    Context* cont = LuaType<Context>::check(L,1);
    LUACHECKOBJ(cont);
    Vector2i* dim = LuaType<Vector2i>::check(L,2);
    cont->SetDimensions(*dim);
    return 0;
}



RegType<Context> ContextMethods[] =
{
    LUAMETHOD(Context,AddEventListener)
    LUAMETHOD(Context,AddMouseCursor)
    LUAMETHOD(Context,CreateDocument)
    LUAMETHOD(Context,LoadDocument)
    LUAMETHOD(Context,LoadMouseCursor)
    LUAMETHOD(Context,Render)
    LUAMETHOD(Context,ShowMouseCursor)
    LUAMETHOD(Context,UnloadAllDocuments)
    LUAMETHOD(Context,UnloadAllMouseCursors)
    LUAMETHOD(Context,UnloadDocument)
    LUAMETHOD(Context,UnloadMouseCursor)
    LUAMETHOD(Context,Update)
    { NULL, NULL },
};

luaL_reg ContextGetters[] =
{
    LUAGETTER(Context,dimensions)
    LUAGETTER(Context,documents)
    LUAGETTER(Context,focus_element)
    LUAGETTER(Context,hover_element)
    LUAGETTER(Context,name)
    LUAGETTER(Context,root_element)
    { NULL, NULL },
};

luaL_reg ContextSetters[] =
{
    LUASETTER(Context,dimensions)
    { NULL, NULL },
};

LUACORETYPEDEFINE(Context,true)
}
}
}