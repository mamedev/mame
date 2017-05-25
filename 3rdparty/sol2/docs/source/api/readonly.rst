readonly
========
routine to mark a member variable as read-only
----------------------------------------------

.. code-block:: cpp
	
	template <typename T>
	auto readonly( T&& value );

The goal of read-only is to protect a variable set on a usertype or a function. Simply wrap it around a member variable, e.g. ``sol::readonly( &my_class::my_member_variable )`` in the appropriate place to use it. If someone tries to set it, it will throw an error. This can ONLY work on :doc:`usertypes<usertype>` and when you specifically set a member variable as a function and wrap it with this. It will NOT work anywhere else: doing so will invoke compiler errors.

If you are looking to make a read-only table, you need to go through a bit of a complicated song and dance by overriding the ``__index`` metamethod. Here's a complete example on the way to do that using ``sol``:


.. code-block:: cpp
	:caption: read-only.cpp

	#define SOL_CHECK_ARGUMENTS
	#include <sol.hpp>

	#include <iostream>

	struct object {
	    void my_func() {
	        std::cout << "hello\n";
	    }
	};

	int deny(lua_State* L) {
	    return luaL_error(L, "HAH! Deniiiiied!");
	}

	int main() {
	    sol::state lua;
	    lua.open_libraries(sol::lib::base);

	    object my_obj;

	    sol::table obj_table = lua.create_named_table("object");

	    sol::table obj_metatable = lua.create_table_with();
	    obj_metatable.set_function("my_func", &object::my_func, &my_obj);
	    // Set whatever else you need to
	    // on the obj_metatable, 
	    // not on the obj_table itself!

	    // Properly self-index metatable to block things
	    obj_metatable[sol::meta_function::new_index] = deny;
	    obj_metatable[sol::meta_function::index] = obj_metatable;

	    // Set it on the actual table
	    obj_table[sol::metatable_key] = obj_metatable;

	    try {
	        lua.script(R"(
	print(object.my_func)
	object["my_func"] = 24
	print(object.my_func)
	        )");
	    }
	    catch (const std::exception& e) {
	        std::cout << e.what() << std::endl;
	    }
	    return 0;
	}

It is a verbose example, but it explains everything. Because the process is a bit involved and can have unexpected consequences for users that make their own tables, making read-only tables is something that we ask the users to do themselves with the above code, as getting the semantics right for the dozens of use cases would be tremendously difficult.
