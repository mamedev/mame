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
 
//#include "precompiled.h"
#include <Rocket/Controls/Controls.h>
#include <Rocket/Core/Core.h>
#include <Rocket/Core/Lua/Utilities.h>

namespace Rocket {
namespace Core {
namespace Lua {
template<typename T>
void LuaType<T>::Register(lua_State* L)
{
    //for annotations, starting at 1, but it is a relative value, not always 1
    lua_newtable(L); //[1] = table
    int methods = lua_gettop(L); //methods = 1

    luaL_newmetatable(L, GetTClassName<T>()); //[2] = metatable named <ClassName>, referred in here by ClassMT
    int metatable = lua_gettop(L); //metatable = 2

    luaL_newmetatable(L, "DO NOT TRASH"); //[3] = metatable named "DO NOT TRASH"
    lua_pop(L,1); //remove the above metatable -> [-1 = 2]

    //store method table in globals so that scripts can add functions written in Lua
    lua_pushvalue(L, methods); //[methods = 1] -> [3] = copy (reference) of methods table
    lua_setglobal(L, GetTClassName<T>()); // -> <ClassName> = [3 = 1], pop top [3]

    //hide metatable from Lua getmetatable()
    lua_pushvalue(L, methods); //[methods = 1] -> [3] = copy of methods table, including modifications above
    lua_setfield(L, metatable, "__metatable"); //[metatable = 2] -> t[k] = v; t = [2 = ClassMT], k = "__metatable", v = [3 = 1]; pop [3]
    
    lua_pushcfunction(L, index); //index = cfunction -> [3] = cfunction
    lua_setfield(L, metatable, "__index"); //[metatable = 2] -> t[k] = v; t = [2], k = "__index", v = cfunc; pop [3]

    lua_pushcfunction(L, newindex);
    lua_setfield(L, metatable, "__newindex");

    lua_pushcfunction(L, gc_T);
    lua_setfield(L, metatable, "__gc");

    lua_pushcfunction(L, tostring_T);
    lua_setfield(L, metatable, "__tostring");

    ExtraInit<T>(L,metatable); //optionally implemented by individual types

    lua_newtable(L); //for method table -> [3] = this table
    lua_setmetatable(L, methods); //[methods = 1] -> metatable for [1] is [3]; [3] is popped off, top = [2]

    _regfunctions(L,metatable,methods);

    lua_pop(L, 2); //remove the two items from the stack, [1 = methods] and [2 = metatable]
}


template<typename T>
int LuaType<T>::push(lua_State *L, T* obj, bool gc)
{
    //for annotations, starting at index 1, but it is a relative number, not always 1
    if (!obj) { lua_pushnil(L); return lua_gettop(L); }
    luaL_getmetatable(L, GetTClassName<T>());  // lookup metatable in Lua registry ->[1] = metatable of <ClassName>
    if (lua_isnil(L, -1)) luaL_error(L, "%s missing metatable", GetTClassName<T>());
    int mt = lua_gettop(L); //mt = 1
    T** ptrHold = (T**)lua_newuserdata(L,sizeof(T**)); //->[2] = empty userdata
    int ud = lua_gettop(L); //ud = 2
    if(ptrHold != NULL)
    {
        *ptrHold = obj; 
        lua_pushvalue(L, mt); // ->[3] = copy of [1]
        lua_setmetatable(L, -2); //[-2 = 2] -> [2]'s metatable = [3]; pop [3]
        char name[32];
        tostring(name,obj);
        lua_getfield(L,LUA_REGISTRYINDEX,"DO NOT TRASH"); //->[3] = value returned from function
        if(lua_isnil(L,-1) ) //if [3] hasn't been created yet, then create it
        {
            luaL_newmetatable(L,"DO NOT TRASH"); //[4] = the new metatable
            lua_pop(L,1); //pop [4]
        }
        lua_pop(L,1); //pop [3]
        lua_getfield(L,LUA_REGISTRYINDEX,"DO NOT TRASH"); //->[3] = value returned from function
        if(gc == false) //if we shouldn't garbage collect it, then put the name in to [3]
        {
            lua_pushboolean(L,1);// ->[4] = true
            lua_setfield(L,-2,name); //represents t[k] = v, [-2 = 3] = t -> v = [4], k = <ClassName>; pop [4]
        }
        else
        {
            if(IsReferenceCounted<T>())
                ((Rocket::Core::ReferenceCountable*)obj)->AddReference();
        }
        lua_pop(L,1); // -> pop [3]
    }
    lua_settop(L,ud); //[ud = 2] -> remove everything that is above 2, top = [2]
    lua_replace(L, mt); //[mt = 1] -> move [2] to pos [1], and pop previous [1]
    lua_settop(L, mt); //remove everything above [1]
    return mt;  // index of userdata containing pointer to T object
}


template<typename T>
T* LuaType<T>::check(lua_State* L, int narg)
{
    T** ptrHold = static_cast<T**>(lua_touserdata(L,narg));
    if(ptrHold == NULL)
        return NULL;
    return (*ptrHold);
}


//private members

template<typename T>
int LuaType<T>::thunk(lua_State* L)
{
    // stack has userdata, followed by method args
    T *obj = check(L, 1);  // get 'self', or if you prefer, 'this'
    lua_remove(L, 1);  // remove self so member function args start at index 1
    // get member function from upvalue
    RegType *l = static_cast<RegType*>(lua_touserdata(L, lua_upvalueindex(1)));
    //at the moment, there isn't a case where NULL is acceptable to be used in the function, so check
    //for it here, rather than individually for each function
    if(obj == NULL)
    {
        lua_pushnil(L);
        return 1;
    }
    else
        return l->func(L,obj);  // call member function
}



template<typename T>
void LuaType<T>::tostring(char* buff, void* obj)
{
    sprintf(buff,"%p",obj);
}


template<typename T>
int LuaType<T>::gc_T(lua_State* L)
{
    T * obj = check(L,1); //[1] = this userdata
    if(obj == NULL)
        return 0;
    lua_getfield(L,LUA_REGISTRYINDEX,"DO NOT TRASH"); //->[2] = return value from this
    if(lua_istable(L,-1) ) //[-1 = 2], if it is a table
    {
        char name[32];
        tostring(name,obj);
        lua_getfield(L,-1, std::string(name).c_str()); //[-1 = 2] -> [3] = the value returned from if <ClassName> exists in the table to not gc
        if(lua_isnil(L,-1) ) //[-1 = 3] if it doesn't exist, then we are free to garbage collect c++ side
        {
            if(IsReferenceCounted<T>())
            {
                ((Rocket::Core::ReferenceCountable*)obj)->RemoveReference();
            }
            else
            {
                delete obj;
                obj = NULL;
            }
        }
    }
    lua_pop(L,3); //balance function
    return 0;
}


template<typename T>
int LuaType<T>::tostring_T(lua_State* L)
{
    char buff[32];
    T** ptrHold = (T**)lua_touserdata(L,1);
    T *obj = *ptrHold;
    sprintf(buff, "%p", obj);
    lua_pushfstring(L, "%s (%s)", GetTClassName<T>(), buff);
    return 1;
}



template<typename T>
int LuaType<T>::index(lua_State* L)
{
    /*the table obj and the missing key are currently on the stack(index 1 & 2) as defined by the Lua language*/
    lua_getglobal(L,GetTClassName<T>()); //stack pos [3] (fairly important, just refered to as [3])
    // string form of the key.
	const char* key = luaL_checkstring(L,2);
    if(lua_istable(L,-1) )  //[-1 = 3]
    {
        lua_pushvalue(L,2); //[2] = key, [4] = copy of key
        lua_rawget(L,-2); //[-2 = 3] -> pop top and push the return value to top [4]
        //If the key were looking for is not in the table, retrieve its' metatables' index value.
        if(lua_isnil(L,-1)) //[-1 = 4] is value from rawget above
        {
            //try __getters
            lua_pop(L,1); //remove top item (nil) from the stack
            lua_pushstring(L, "__getters");
            lua_rawget(L,-2); //[-2 = 3], <ClassName>._getters -> result to [4]
            lua_pushvalue(L,2); //[2 = key] -> copy to [5]
            lua_rawget(L,-2); //[-2 = __getters] -> __getters[key], result to [5]
            if(lua_type(L,-1) == LUA_TFUNCTION) //[-1 = 5]
            {
                lua_pushvalue(L,1); //push the userdata to the stack [6]
                if(lua_pcall(L,1,1,0) != 0) //remove one, result is at [6]
                    Report(L, String(GetTClassName<T>()).Append(".__index for ").Append(lua_tostring(L,2)).Append(": "));
            }
            else
            {
                lua_settop(L,4); //forget everything we did above
                lua_getmetatable(L,-2); //[-2 = 3] -> metatable from <ClassName> to top [5]
                if(lua_istable(L,-1) ) //[-1 = 5] = the result of the above
                {
                    lua_getfield(L,-1,"__index"); //[-1 = 5] = check the __index metamethod for the metatable-> push result to [6]
                    if(lua_isfunction(L,-1) ) //[-1 = 6] = __index metamethod
                    {
                        lua_pushvalue(L,1); //[1] = object -> [7] = object
                        lua_pushvalue(L,2); //[2] = key -> [8] = key
                        if(lua_pcall(L,2,1,0) != 0) //call function at top of stack (__index) -> pop top 2 as args; [7] = return value
                            Report(L, String(GetTClassName<T>()).Append(".__index for ").Append(lua_tostring(L,2)).Append(": "));
                    }
                    else if(lua_istable(L,-1) )
                        lua_getfield(L,-1,key); //shorthand version of above -> [7] = return value
                    else
                        lua_pushnil(L); //[7] = nil
                }
                else
                    lua_pushnil(L); //[6] = nil
            }
        }
        else if(lua_istable(L,-1) )//[-1 = 4] is value from rawget [3]
        {
            lua_pushvalue(L,2); //[2] = key, [5] = key
            lua_rawget(L,-2); //[-2 = 3] = table of <ClassName> -> pop top and push the return value to top [5]
        }
    }
    else
        lua_pushnil(L); //[4] = nil

    lua_insert(L,1); //top element to position 1 -> [1] = top element as calculated in the earlier rest of the function
    lua_settop(L,1); // -> [1 = -1], removes the other elements
    return 1;
}



template<typename T>
int LuaType<T>::newindex(lua_State* L)
{
    //[1] = obj, [2] = key, [3] = value
    //look for it in __setters
    lua_getglobal(L,GetTClassName<T>()); //[4] = this table
    lua_pushstring(L,"__setters"); //[5]
    lua_rawget(L,-2); //[-2 = 4] -> <ClassName>.__setters to [5]
    lua_pushvalue(L,2); //[2 = key] -> [6] = copy of key
    lua_rawget(L,-2); //[-2 = __setters] -> __setters[key] to [6]
    if(lua_type(L,-1) == LUA_TFUNCTION)
    {
        lua_pushvalue(L,1); //userdata at [7]
        lua_pushvalue(L,3); //[8] = copy of [3]
        if(lua_pcall(L,2,0,0) != 0) //call function, pop 2 off push 0 on
            Report(L, String(GetTClassName<T>()).Append(".__newindex for ").Append(lua_tostring(L,2)).Append(": ")); 
    }
    else
        lua_pop(L,1); //not a setter function.
    lua_pop(L,2); //pop __setters and the <Classname> table
    return 0;
}


template<typename T>
void LuaType<T>::_regfunctions(lua_State* L, int meta, int methods)
{
    //fill method table with methods.
    for(RegType* m = (RegType*)GetMethodTable<T>(); m->name; m++)
    {
        lua_pushstring(L, m->name); // ->[1] = name of function Lua side
        lua_pushlightuserdata(L, (void*)m); // ->[2] = pointer to the object containing the name and the function pointer as light userdata
        lua_pushcclosure(L, thunk, 1); //thunk = function pointer -> pop 1 item from stack, [2] = closure
        lua_settable(L, methods); // represents t[k] = v, t = [methods] -> pop [2 = closure] to be v, pop [1 = name] to be k
    }

    
    lua_getfield(L,methods, "__getters"); // -> table[1]
    if(lua_isnoneornil(L,-1))
    {
        lua_pop(L,1); //pop unsuccessful get
        lua_newtable(L); // -> table [1]
        lua_setfield(L,methods,"__getters"); // pop [1]
        lua_getfield(L,methods,"__getters"); // -> table [1]
    }
    for(luaL_reg* m = (luaL_reg*)GetAttrTable<T>(); m->name; m++)
    {
        lua_pushcfunction(L,m->func); // -> [2] is this function
        lua_setfield(L,-2,m->name); //[-2 = 1] -> __getters.name = function
    }
    lua_pop(L,1); //pop __getters


    lua_getfield(L,methods, "__setters"); // -> table[1]
    if(lua_isnoneornil(L,-1))
    {
        lua_pop(L,1); //pop unsuccessful get
        lua_newtable(L); // -> table [1]
        lua_setfield(L,methods,"__setters"); // pop [1]
        lua_getfield(L,methods,"__setters"); // -> table [1]
    }
    for(luaL_reg* m = (luaL_reg*)SetAttrTable<T>(); m->name; m++)
    {
        lua_pushcfunction(L,m->func); // -> [2] is this function
        lua_setfield(L,-2,m->name); //[-2 = 1] -> __setters.name = function
    }
    lua_pop(L,1); //pop __setters
}

}
}
}
