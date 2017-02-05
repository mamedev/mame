resolve
=======
utility to pick overloaded C++ function calls
---------------------------------------------

.. code-block:: cpp
	:caption: function: resolve C++ overload

	template <typename... Args, typename F>
	constexpr auto resolve( F f );

``resolve`` is a function that is meant to help users pick a single function out of a group of overloaded functions in C++. It works for *both member and free functions* You can use it to pick overloads by specifying the signature as the first template argument. Given a collection of overloaded functions:

.. code-block:: cpp
	:linenos:

	int overloaded(int x);
	int overloaded(int x, int y);
	int overloaded(int x, int y, int z);

	struct thing {
		int overloaded() const;
		int overloaded(int x);
		int overloaded(int x, int y);
		int overloaded(int x, int y, int z);
	};

You can disambiguate them using ``resolve``:

..  code-block:: cpp
	:linenos:

	auto one_argument_func = resolve<int(int)>( overloaded );
	auto two_argument_func = resolve<int(int, int)>( overloaded );
	auto three_argument_func = resolve<int(int, int, int)>( overloaded );
	auto member_three_argument_func = resolve<int(int, int, int)>( &thing::overloaded );
	auto member_zero_argument_const_func = resolve<int() const>( &thing::overloaded );

It is *important* to note that ``const`` is placed at the end for when you desire const overloads. You will get compiler errors if you are not specific and do not properly disambiguate for const member functions. This resolution also becomes useful when setting functions on a :doc:`table<table>` or :doc:`state_view<state>`:

..  code-block:: cpp
	:linenos:

	sol::state lua;

	lua.set_function("a", resolve<int(int)>( overloaded ) );
	lua.set_function("b", resolve<int(int, int)>( overloaded ));
	lua.set_function("c", resolve<int(int, int, int)>( overloaded ));


It can also be used with :doc:`sol::c_call<c_call>`:

.. code-block:: cpp
	:linenos:

	sol::state lua;
	
	auto f = sol::c_call<
		decltype(sol::resolve<int(int, int)>(&overloaded)), 
		sol::resolve<int(int, int)>(&overloaded)
	>;
	lua.set_function("f", f);
	
	lua.script("f(1, 2)");

