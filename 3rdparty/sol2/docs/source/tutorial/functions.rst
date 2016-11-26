functions and You
=================

Sol can register all kinds of functions. Many are shown in the :doc:`quick 'n' dirty<all-the-things>`, but here we will discuss many of the additional ways you can register functions into a sol-wrapped Lua system.

Setting a new function
----------------------

Given a C++ function, you can drop it into Sol in several equivalent ways, working similar to how :ref:`setting variables<writing-main-cpp>` works:

.. code-block:: cpp
	:linenos:
	:caption: Registering C++ functions
	:name: writing-functions

	#include <sol.hpp>

	std::string my_function( int a, std::string b ) {
		// Create a string with the letter 'D' "a" times,
		// append it to 'b'
		return b + std::string( 'D', a ); 
	}

	int main () {

		sol::state lua;

		lua["my_func"] = my_function; // way 1
		lua.set("my_func", my_function); // way 2
		lua.set_function("my_func", my_function); // way 3

		// This function is now accessible as 'my_func' in
		// lua scripts / code run on this state:
		lua.script("some_str = my_func(1, "Da");

		// Read out the global variable we stored in 'some_str' in the
		// quick lua code we just executed
		std::string some_str = lua["some_str"];
		// some_str == "DaD"
	}

The same code works with all sorts of functions, from member function/variable pointers you have on a class as well as lambdas:

.. code-block:: cpp
	:linenos:
	:caption: Registering C++ member functions
	:name: writing-member-functions

	struct my_class {
		int a = 0;

		my_class(int x) : a(x) {

		}

		int func() {
			++a; // increment a by 1
			return a;
		}
	};

	int main () {

		sol::state lua;

		// Here, we are binding the member function and a class instance: it will call the function on
		// the given class instance
		lua.set_function("my_class_func", &my_class::func, my_class());

		// We do not pass a class instance here:
		// the function will need you to pass an instance of "my_class" to it
		// in lua to work, as shown below
		lua.set_function("my_class_func_2", &my_class::func);

		// With a pre-bound instance:
		lua.script(R"(
			first_value = my_class_func()
			second_value = my_class_func()
		)");
		// first_value == 1
		// second_value == 2

		// With no bound instance:
		lua.set("obj", my_class(24));
		// Calls "func" on the class instance
		// referenced by "obj" in Lua
		lua.script(R"(
			third_value = my_class_func_2(obj)
			fourth_value = my_class_func_2(obj)
		)");
		// first_value == 25
		// second_value == 26
	}

Member class functions and member class variables will both be turned into functions when set in this manner. You can get intuitive variable with the ``obj.a = value`` access after this section when you learn about :doc:`usertypes to have C++ in Lua<cxx-in-lua>`, but for now we're just dealing with functions!


Another question a lot of people have is about function templates. Function templates -- member functions or free functions -- cannot be registered because they do not exist until you instantiate them in C++. Therefore, given a templated function such as:

.. code-block:: cpp
	:linenos:
	:caption: A C++ templated function
	:name: writing-templated-functions-the-func

	template <typename A, typename B>
	auto my_add( A a, B b ) {
		return a + b; 
	}


You must specify all the template arguments in order to bind and use it, like so:

.. code-block:: cpp
	:linenos:
	:caption: Registering function template instantiations
	:name: writing-templated-functions


	int main () {

		sol::state lua;

		// adds 2 integers
		lua["my_int_add"] = my_add<int, int>;
		
		// concatenates 2 strings
		lua["my_string_combine"] = my_add<std::string, std::string>;

		lua.script("my_num = my_int_add(1, 2)");
		int my_num = lua["my_num"];
		// my_num == 3
		
		lua.script("my_str = my_string_combine('bark bark', ' woof woof')");
		std::string my_str = lua["my_str"];
		// my_str == "bark bark woof woof"
	}

Notice here that we bind two separate functions. What if we wanted to bind only one function, but have it behave differently based on what arguments it is called with? This is called Overloading, and it can be done with :doc:`sol::overload<../api/overload>` like so:

.. code-block:: cpp
	:linenos:
	:caption: Registering C++ function template instantiations
	:name: writing-templated-functions-overloaded


	int main () {

		sol::state lua;

		// adds 2 integers
		lua["my_combine"] = sol::overload( my_add<int, int>, my_add<std::string, std::string> );
		
		lua.script("my_num = my_combine(1, 2)");
		lua.script("my_str = my_combine('bark bark', ' woof woof')");
		int my_num = lua["my_num"];
		std::string my_str = lua["my_str"];
		// my_num == 3
		// my_str == "bark bark woof woof"
	}

This is useful for functions which can take multiple types and need to behave differently based on those types. You can set as many overloads as you want, and they can be of many different types.

.. note::

	Function object ``obj`` -- a struct with a ``return_type operator()( ... )`` member defined on them, like all C++ lambdas -- are not interpreted as functions when you use ``set`` for ``mytable.set( key, value )``. This only happens automagically with ``mytable[key] = obj``. To be explicit about wanting a struct to be interpreted as a function, use ``mytable.set_function( key, func_value );``. You can be explicit about wanting a function as well by using the :doc:`sol::as_function<../api/as_function>` call.


Getting a function from Lua
---------------------------

There are 2 ways to get a function from Lua. One is with :doc:`sol::function<../api/function>` and the other is a more advanced wrapper with :doc:`sol::protected_function<../api/protected_function>`. Use them to retrieve callables from Lua and call the underlying function, in two ways:

.. code-block:: cpp
	:linenos:
	:caption: Retrieving a sol::function 
	:name: reading-functions

	int main () {

		sol::state lua;

		lua.script(R"(
			function f (a)
				return a + 5
			end
		)");

		// Get and immediately call
		int x = lua["f"](30);
		// x == 35

		// Store it into a variable first, then call
		sol::function f = lua["f"];
		int y = f(20);
		// y == 25
	}

You can get anything that's a callable in Lua, including C++ functions you bind using ``set_function`` or similar. ``sol::protected_function`` behaves similarly to ``sol::function``, but has a :ref:`error_handler<protected-function-error-handler>` variable you can set to a Lua function. This catches all errors and runs them through the error-handling function:


.. code-block:: cpp
	:linenos:
	:caption: Retrieving a sol::protected_function 
	:name: reading-protected-functions

	int main () {
		sol::state lua;

		lua.script(R"(
			function handler (message)
				return "Handled this message: " .. message
			end

			function f (a)
				if a < 0 then
					error("negative number detected")
				end
				return a + 5
			end
		)");

		sol::protected_function f = lua["f"];
		f.error_handler = lua["handler"];

		sol::protected_function_result result = f(-500);
		if (result.valid()) {
			// Call succeeded
			int x = result;
		}
		else {
			// Call failed
			sol::error err = result;
			std::string what = err.what();
			// 'what' Should read 
			// "Handled this message: negative number detected"
		} 
	}


Multiple returns to and from Lua
--------------------------------

You can return multiple items to and from Lua using ``std::tuple``/``std::pair`` classes provided by C++. These enable you to also use :doc:`sol::tie<../api/tie>` to set return values into pre-declared items. To recieve multiple returns, just ask for a ``std::tuple`` type from the result of a function's computation, or ``sol::tie`` a bunch of pre-declared variables together and set the result equal to that:

.. code-block:: cpp
	:linenos:
	:caption: Multiple returns from Lua 
	:name: multi-return-lua-functions	

	int main () {
		sol::state lua;

		lua.script("function f (a, b, c) return a, b, c end");
		
		std::tuple<int, int, int> result;
		result = lua["f"](1, 2, 3); 
		// result == { 1, 2, 3 }
		int a, int b;
		std::string c;
		sol::tie( a, b, c ) = lua["f"](1, 2, "bark");
		// a == 1
		// b == 2
		// c == "bark"
	}

You can also return mutiple items yourself from a C++-bound function. Here, we're going to bind a C++ lambda into Lua, and then call it through Lua and get a ``std::tuple`` out on the other side:

.. code-block:: cpp	
	:linenos:
	:caption: Multiple returns into Lua 
	:name: multi-return-cxx-functions	

	int main () {
		sol::state lua;

		lua["f"] = [](int a, int b, sol::object c) {
			// sol::object can be anything here: just pass it through
			return std::make_tuple( a, b, c );
		};
		
		std::tuple<int, int, int> result = lua["f"](1, 2, 3); 
		// result == { 1, 2, 3 }
		
		std::tuple<int, int, std::string> result2;
		result2 = lua["f"](1, 2, "Arf?")
		// result2 == { 1, 2, "Arf?" }

		int a, int b;
		std::string c;
		sol::tie( a, b, c ) = lua["f"](1, 2, "meow");
		// a == 1
		// b == 2
		// c == "meow"
	}


Note here that we use :doc:`sol::object<../api/object>` to transport through "any value" that can come from Lua. You can also use ``sol::make_object`` to create an object from some value, so that it can be returned into Lua as well.


Any return to and from Lua
--------------------------

It was hinted at in the previous code example, but ``sol::object`` is a good way to pass "any type" back into Lua (while we all wait for ``std::variant<...>`` to get implemented and shipped by C++ compiler/library implementers).

It can be used like so, inconjunction with ``sol::this_state``:

.. code-block:: cpp	
	:linenos:
	:caption: Return anything into Lua 
	:name: object-return-cxx-functions	

	sol::object fancy_func (sol::object a, sol::object b, sol::this_state s) {
		sol::state_view lua(s);
		if (a.is<int>() && b.is<int>()) {
			return sol::make_object(lua, a.as<int>() + b.as<int>());
		}
		else if (a.is<bool>()) {
			bool do_triple = a.as<bool>();
			return sol::make_object(lua, b.as<double>() * ( do_triple ? 3 : 1 ) );
		}
		return sol::make_object(lua, sol::nil);
	}

	int main () {
		sol::state lua;

		lua["f"] = fancy_func;
		
		int result = lua["f"](1, 2);
		// result == 3
		double result2 = lua["f"](false, 2.5);
		// result2 == 2.5

		// call in Lua, get result
		lua.script("result3 = f(true, 5.5)");
		double result3 = lua["result3"];
		// result3 == 16.5
	}


This covers almost everything you need to know about Functions and how they interact with Sol. For some advanced tricks and neat things, check out :doc:`sol::this_state<../api/this_state>` and :doc:`sol::variadic_args<../api/variadic_args>`. The next stop in this tutorial is about :doc:`C++ types (usertypes) in Lua<cxx-in-lua>`! If you need a bit more information about functions in the C++ side and how to best utilize arguments from C++, see :ref:`this note<function-argument-handling>`.