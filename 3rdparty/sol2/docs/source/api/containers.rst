containers
==========
for handling ``std::vector/map/set`` and others
-----------------------------------------------

Sol2 automatically converts containers (detected using the ``sol::is_container<T>`` type trait, which simply looks for begin / end) to be a special kind of userdata with metatable on it. For Lua 5.2 and 5.3, this is extremely helpful as you can make typical containers behave like Lua tables without losing the actual container that they came from, as well as a small amount of indexing and other operations that behave properly given the table type.


a complete example
------------------

Here's a complete working example of it working for Lua 5.3 and Lua 5.2, and how you can retrieve out the container in all versions:

.. code-block:: cpp
	:caption: containers.cpp

	#define SOL_CHECK_ARGUMENTS
	#include <sol.hpp>

	int main() {
		sol::state lua;
		lua.open_libraries();

		lua.script(R"(
		function f (x)
			print('--- Calling f ---')
			for k, v in pairs(x) do
				print(k, v)
			end
		end
		)");

		// Have the function we 
		// just defined in Lua
		sol::function f = lua["f"];

		// Set a global variable called 
		// "arr" to be a vector of 5 lements
		lua["arr"] = std::vector<int>{ 2, 4, 6, 8, 10 };
		
		// Call it, see 5 elements
		// printed out
		f(lua["arr"]);

		// Mess with it in C++
		std::vector<int>& reference_to_arr = lua["arr"];
		reference_to_arr.push_back(12);

		// Call it, see *6* elements
		// printed out
		f(lua["arr"]);

		return 0;
	}


Note that this will not work well in Lua 5.1, as it has explicit table checks and does not check metamethods, even when ``pairs`` or ``ipairs`` is passed a table. In that case, you will need to use a more manual iteration scheme or you will have to convert it to a table. In C++, you can use :doc:`sol::as_table<as_table>` when passing something to the library to get a table out of it: ``lua["arr"] = as_table( std::vector<int>{ ... });``. For manual iteration in Lua code without using ``as_table`` for something with indices, try:

.. code-block:: lua
	:caption: iteration.lua

	for i = 1, #vec do
		print(i, vec[i]) 
	end

There are also other ways to iterate over key/values, but they can be difficult due to not having proper support in Lua 5.1. We recommend that you upgrade to Lua 5.2 or 5.3.


additional functions
--------------------

Based on the type pushed, a few additional functions are added as "member functions" (``self`` functions called with ``obj:func()`` or ``obj.func(obj)`` syntax) within a Lua script:

* ``my_container:clear()``: This will call the underlying containers ``clear`` function.
* ``my_container:add( key, value )`` or ``my_container:add( value )``: this will add to the end of the container, or if it is an associative or ordered container, simply put in an expected key-value pair into it.
* ``my_contaner:insert( where, value )`` or ``my_contaner:insert( key, value )``: similar to add, but it only takes two arguments. In the case of ``std::vector`` and the like, the first argument is a ``where`` integer index. The second argument is the value. For associative containers, a key and value argument are expected.


.. _container-detection:

too-eager container detection?
------------------------------


If you have a type that has ``begin`` or ``end`` member functions but don't provide iterators, you can specialize ``sol::is_container<T>`` to be ``std::false_type``, and that will treat the type as a regular usertype and push it as a regular userdata:

.. code-block:: cpp
	:caption: specialization.hpp

	struct not_container {
		void begin() {

		}

		void end() {

		}
	};

	namespace sol {
		template <>
		struct is_container<not_container> : std::false_type {};
	}