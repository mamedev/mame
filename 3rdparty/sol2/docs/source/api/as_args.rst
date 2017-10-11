as_args
=======
turn an iterable argument into multiple arguments
-------------------------------------------------

.. code-block:: cpp
	
	template <typename T>
	as_args_t { ... };

	template <typename T>
	as_args_t<T> as_args( T&& );


``sol::as_args`` is a function that that takes an iterable and turns it into multiple arguments to a function call. It forwards its arguments, and is meant to be used as shown below:

.. code-block:: cpp
	:caption: as_args.c++

	#define SOL_CHECK_ARGUMENTS
	#include <sol.hpp>

	#include <vector>
	#include <set>

	int main(int argc, const char* argv[]) {
		
		sol::state lua;
		lua.open_libraries();

		lua.script("function f (a, b, c, d) print(a, b, c, d) end");

		sol::function f = lua["f"];

		std::vector<int> v2{ 3, 4 };
		f(1, 2, sol::as_args(v2));

		std::set<int> v4{ 3, 1, 2, 4 };
		f(sol::as_args(v4));

		int v3[] = { 2, 3, 4 };
		f(1, sol::as_args(v3));

		return 0;
	}


It is basically implemented as a `one-way customization point`_. For more information about customization points, see the :doc:`tutorial on how to customize Sol to work with your types<../tutorial/customization>`.

.. _one-way customization point: https://github.com/ThePhD/sol2/blob/develop/sol/as_args.hpp
