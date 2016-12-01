this_state
==========
transparent state argument for the current state
------------------------------------------------

.. code-block:: cpp
	
	struct this_state;

This class is a transparent type that is meant to be gotten in functions to get the current lua state a bound function or usertype method is being called from. It does not actually retrieve anything from lua nor does it increment the argument count, making it "invisible" to function calls in lua and calls through ``std::function<...>`` and :doc:`sol::function<function>` on this type. It can be put in any position in the argument list of a function:

.. code-block:: cpp
	:linenos:

	sol::state lua;
    
	lua.set_function("bark", []( sol::this_state s, int a, int b ){
		lua_State* L = s; // current state
		return a + b + lua_gettop(L);
	});
	
	lua.script("first = bark(2, 2)"); // only takes 2 arguments, NOT 3
		
	// Can be at the end, too, or in the middle: doesn't matter
	lua.set_function("bark", []( int a, int b, sol::this_state s ){
		lua_State* L = s; // current state
		return a + b + lua_gettop(L);
	});

	lua.script("second = bark(2, 2)"); // only takes 2 arguments
		