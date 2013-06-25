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
 
#ifndef ROCKETCORELUALUATYPE_H
#define ROCKETCORELUALUATYPE_H

#include <Rocket/Core/Lua/Header.h>
#include <Rocket/Core/Lua/lua.hpp>


//As an example, if you used this macro like
//LUAMETHOD(Unit,GetId)
//it would result in code that looks like
//{ "GetId", UnitGetId },
//Which would force you to create a global function named UnitGetId in C with the correct function signature, usually int(*)(lua_State*,type*);
#define LUAMETHOD(type,name) { #name, type##name },

//See above, but the method must match the function signature int(*)(lua_State*) and as example:
//LUAGETTER(Unit,Id) would mean you need a function named UnitGetAttrId
//The first stack position will be the userdata
#define LUAGETTER(type,varname) { #varname, type##GetAttr##varname },

//Same method signature as above, but as example:
//LUASETTER(Unit,Id) would mean you need a function named UnitSetAttrId
//The first stack position will be the userdata, and the second will be value on the other side of the equal sign
#define LUASETTER(type,varname) { #varname, type##SetAttr##varname },

#define CHECK_BOOL(L,narg) (lua_toboolean((L),(narg)) > 0 ? true : false )
#define LUACHECKOBJ(obj) if((obj) == NULL) { lua_pushnil(L); return 1; }

 /** Used to remove repetitive typing at the cost of flexibility. When you use this, you @em must have 
 functions with the same name as defined in the macro. For example, if you used @c Element as type, you would
 have to have functions named @c ElementMethods, @c ElementGetters, @c ElementSetters that return the appropriate
 types.
 @param is_reference_counted true if the type inherits from Rocket::Core::ReferenceCountable, false otherwise*/
#define LUACORETYPEDEFINE(type,is_ref_counted) \
    template<> const char* GetTClassName<type>() { return #type; } \
    template<> RegType<type>* GetMethodTable<type>() { return type##Methods; } \
    template<> luaL_reg* GetAttrTable<type>() { return type##Getters; } \
    template<> luaL_reg* SetAttrTable<type>() { return type##Setters; } \
    template<> bool IsReferenceCounted<type>() { return (is_ref_counted); } \

//We can't use LUACORETYPEDEFINE due to namespace issues
#define LUACONTROLSTYPEDEFINE(type,is_ref_counted) \
    template<> const char* GetTClassName<type>() { return #type; } \
    template<> RegType<type>* GetMethodTable<type>() { return Rocket::Controls::Lua::type##Methods; } \
    template<> luaL_reg* GetAttrTable<type>() { return Rocket::Controls::Lua::type##Getters; } \
    template<> luaL_reg* SetAttrTable<type>() { return Rocket::Controls::Lua::type##Setters; } \
    template<> bool IsReferenceCounted<type>() { return (is_ref_counted); } \

/** Used to remove repetitive typing at the cost of flexibility. It creates function prototypes for
getting the name of the type, method tables, and if it is reference counted.
When you use this, you either must also use
the LUACORETYPEDEFINE macro, or make sure that the function signatures are @em exact.*/
#define LUACORETYPEDECLARE(type) \
    template<> ROCKETLUA_API const char* GetTClassName<type>(); \
    template<> ROCKETLUA_API RegType<type>* GetMethodTable<type>(); \
    template<> ROCKETLUA_API luaL_reg* GetAttrTable<type>(); \
    template<> ROCKETLUA_API luaL_reg* SetAttrTable<type>(); \
    template<> ROCKETLUA_API bool IsReferenceCounted<type>(); \

/** Used to remove repetitive typing at the cost of flexibility. It creates function prototypes for
getting the name of the type, method tables, and if it is reference counted.
When you use this, you either must also use
the LUACORETYPEDEFINE macro, or make sure that the function signatures are @em exact.*/
#define LUACONTROLSTYPEDECLARE(type) \
    template<> ROCKETLUA_API const char* GetTClassName<type>(); \
    template<> ROCKETLUA_API RegType<type>* GetMethodTable<type>(); \
    template<> ROCKETLUA_API luaL_reg* GetAttrTable<type>(); \
    template<> ROCKETLUA_API luaL_reg* SetAttrTable<type>(); \
    template<> ROCKETLUA_API bool IsReferenceCounted<type>(); \

namespace Rocket {
namespace Core {
namespace Lua {
//replacement for luaL_reg that uses a different function pointer signature, but similar syntax
template<typename T>
struct ROCKETLUA_API RegType
{
    const char* name;
    int (*ftnptr)(lua_State*,T*);
};

/** For all of the methods available from Lua that call to the C functions. */
template<typename T> ROCKETLUA_API RegType<T>* GetMethodTable();
/** For all of the function that 'get' an attribute/property */
template<typename T> ROCKETLUA_API luaL_reg* GetAttrTable();
/** For all of the functions that 'set' an attribute/property  */
template<typename T> ROCKETLUA_API luaL_reg* SetAttrTable();
/** String representation of the class */
template<typename T> ROCKETLUA_API const char* GetTClassName();
/** bool for if it is reference counted */
template<typename T> ROCKETLUA_API bool IsReferenceCounted();
/** gets called from the LuaType<T>::Register function, right before @c _regfunctions.
If you want to inherit from another class, in the function you would want
to call @c _regfunctions<superclass>, where method is metatable_index - 1. Anything
that has the same name in the subclass will be overwrite whatever had the 
same name in the superclass.    */
template<typename T> ROCKETLUA_API void ExtraInit(lua_State* L, int metatable_index);

/**
    This is mostly the definition of the Lua userdata that C++ gives to the user, plus
    some helper functions.

    @author Nathan Starkey
*/
template<typename T>
class ROCKETLUA_API LuaType
{
public:
    typedef int (*ftnptr)(lua_State* L, T* ptr);
    /** replacement for luaL_reg that uses a different function pointer signature, but similar syntax */
    typedef struct { const char* name; ftnptr func; } RegType;

    /** Registers the type T as a type in the Lua global namespace by creating a
     metatable with the same name as the class, setting the metatmethods, and adding the 
     functions from _regfunctions */
    static inline void Register(lua_State *L);
    /** Pushes on to the Lua stack a userdata representing a pointer of T
    @param obj[in] The object to push to the stack
    @param gc[in] If the obj should be deleted or decrease reference count upon the garbage collection
    metamethod being called from the object in Lua
    @return Position on the stack where the userdata resides   */
    static inline int push(lua_State *L, T* obj, bool gc=false);
    /** Statically casts the item at the position on the Lua stack
    @param narg[in] Position of the item to cast on the Lua stack
    @return A pointer to an object of type T or @c NULL   */
    static inline T* check(lua_State* L, int narg);

    /** For calling a C closure with upvalues. Used by the functions defined by RegType
    @return The value that RegType.func returns   */
    static inline int thunk(lua_State* L);
    /** String representation of the pointer. Called by the __tostring metamethod  */
    static inline void tostring(char* buff, void* obj);
    //these are metamethods
    /** The __gc metamethod. If the object was pushed by push(lua_State*,T*,bool) with the third
    argument as true, it will either decrease the reference count or call delete depending on if
    the type is reference counted. If the third argument to push was false, then this does nothing.
    @return 0, since it pushes nothing on to the stack*/
    static inline int gc_T(lua_State* L);
    /** The __tostring metamethod.
    @return 1, because it pushes a string representation of the userdata on to the stack  */
    static inline int tostring_T(lua_State* L);
    /** The __index metamethod. Called whenever the user attempts to access a variable that is
    not in the immediate table. This handles the method calls and calls tofunctions in __getters    */
    static inline int index(lua_State* L);
    /** The __newindex metamethod. Called when the user attempts to set a variable that is not
    int the immediate table. This handles the calls to functions in __setters  */
    static inline int newindex(lua_State* L);
	
    /** Registers methods,getters,and setters to the type. In Lua, the methods exist in the Type name's 
    metatable, and the getters exist in __getters and setters in __setters. The reason for __getters and __setters
    is to have the objects use a 'dot' syntax for properties and a 'colon' syntax for methods.*/
    static inline void _regfunctions(lua_State* L, int meta, int method);
private:
    LuaType(); //hide constructor

};


}
}
}

#include "LuaType.inl" 
#endif