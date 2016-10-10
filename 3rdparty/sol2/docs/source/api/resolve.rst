resolve
=======
utility to pick overloaded C++ function calls
---------------------------------------------

.. code-block:: cpp
	:caption: function: resolve C++ overload

	template <typename... Args, typename F>
	auto resolve( F f );

``resolve`` is a function that is meant to help users pick a single function out of a group of overloaded functions in C++. You can use it to pick overloads by specifying the signature as the first template argument. Given a collection of overloaded functions:

.. code-block:: cpp
	:linenos:

	int overloaded(int x);
	int overloaded(int x, int y);
	int overloaded(int x, int y, int z);

You can disambiguate them using ``resolve``:

..  code-block:: cpp
	:linenos:

	auto one_argument_func = resolve<int(int)>( overloaded );
	auto two_argument_func = resolve<int(int, int)>( overloaded );
	auto three_argument_func = resolve<int(int, int, int)>( overloaded );

This resolution becomes useful when setting functions on a :doc:`table<table>` or :doc:`state_view<state>`:

..  code-block:: cpp
	:linenos:

	sol::state lua;

	lua.set_function("a", resolve<int(int)>( overloaded ) );
	lua.set_function("b", resolve<int(int, int)>( overloaded ));
	lua.set_function("c", resolve<int(int, int, int)>( overloaded ));
