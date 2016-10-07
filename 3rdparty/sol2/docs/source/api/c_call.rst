c_call
======
Templated type to transport functions through templates
-------------------------------------------------------

.. code-block:: cpp
	
	template <typename Function, Function f>
	int c_call (lua_State* L);

	template <typename... Functions>
	int c_call (lua_State* L);

The goal of ``sol::c_call<...>`` is to provide a way to wrap a function and transport it through a compile-time context. This enables faster speed at the cost of a much harder to read / poorer interface. ``sol::c_call`` expects a type for its first template argument, and a value of the previously provided type for the second template argument. To make a compile-time transported overloaded function, specify multiple functions in the same ``type, value`` pairing, but put it inside of a ``sol::wrap``. Note that is can also be placed into the argument list for a :doc:`usertype<usertype>` as well. 

It is advisable for the user to consider making a macro to do the necessary ``decltype( &function_name, ), function_name``. Sol does not provide one because many codebases already have `one similar to this`_.

Here's an example below of various ways to use ``sol::c_call``:

.. code-block:: cpp
	:linenos:
	:caption: Compile-time transported function calls

	#include "sol.hpp"

	int f1(int) { return 32; }
	int f2(int, int) { return 1; }
	struct fer {
		double f3(int, int) {
			return 2.5;
		}
	};


	int main() {

		sol::state lua;
		// overloaded function f
		lua.set("f", sol::c_call<sol::wrap<decltype(&f1), &f1>, sol::wrap<decltype(&f2), &f2>, sol::wrap<decltype(&fer::f3), &fer::f3>>);
		// singly-wrapped function
		lua.set("g", sol::c_call<sol::wrap<decltype(&f1), &f1>>);
		// without the 'sol::wrap' boilerplate
		lua.set("h", sol::c_call<decltype(&f2), &f2>);
		// object used for the 'fer' member function call
		lua.set("obj", fer());

		// call them like any other bound function
		lua.script("r1 = f(1)");
		lua.script("r2 = f(1, 2)");
		lua.script("r3 = f(obj, 1, 2)");
		lua.script("r4 = g(1)");
		lua.script("r5 = h(1, 2)");

		// get the results and see
		// if it worked out
		int r1 = lua["r1"];
		// r1 == 32
		int r2 = lua["r2"];
		// r2 == 1
		double r3 = lua["r3"];
		// r3 == 2.5
		int r4 = lua["r4"];
		// r4 == 32
		int r5 = lua["r5"];
		// r5 == 1

		return 0;
	}


.. _one similar to this: http://stackoverflow.com/a/5628222/5280922
