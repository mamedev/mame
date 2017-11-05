as_function
===========
make sure an object is pushed as a function
-------------------------------------------

.. code-block:: cpp
	
	template <typename Sig = sol::function_sig<>, typename... Args>
	function_argumants<Sig, Args...> as_function ( Args&& ... );

This function serves the purpose of ensuring that a callable struct (like a lambda) can be passed to the ``set( key, value )`` calls on :ref:`sol::table<set-value>` and be treated like a function binding instead of a userdata. It is recommended that one uses the :ref:`sol::table::set_function<set-function>` call instead, but if for some reason one must use ``set``, then ``as_function`` can help ensure a callable struct is handled like a lambda / callable, and not as just a userdata structure.

This class can also make it so usertypes bind variable types as functions to for usertype bindings.

.. code-block:: cpp

	#include <sol.hpp>

	int main () {
		struct callable {
			int operator()( int a, bool b ) {
				return a + b ? 10 : 20;
			}
		};


		sol::state lua;
		// Binds struct as userdata
		lua.set( "not_func", callable() );
		// Binds struct as function
		lua.set( "func", sol::as_function( callable() ) );
		// equivalent: lua.set_function( "func", callable() );
		// equivalent: lua["func"] = callable();
	}

Note that if you actually want a userdata, but you want it to be callable, you simply need to create a :ref:`sol::table::new_usertype<new-usertype>` and then bind the ``"__call"`` metamethod (or just use ``sol::meta_function::call`` :ref:`enumeration<meta_function_enum>`).

Here's an example of binding a variable as a function to a usertype:

.. code-block:: cpp

	#include <sol.hpp>

	int main () {
		class B {
		public:
			int bvar = 24;
		};

		sol::state lua;
		lua.open_libraries();
		lua.new_usertype<B>("B", 
			// bind as variable
			"b", &B::bvar,
			// bind as function
			"f", sol::as_function(&B::bvar)
		);

		B b;
		lua.set("b", &b);
		lua.script("x = b:f()");
		lua.script("y = b.b");
		int x = lua["x"];
		int y = lua["y"];
		assert(x == 24);
		assert(y == 24);
	}
