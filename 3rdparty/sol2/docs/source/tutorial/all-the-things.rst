tutorial: quick 'n' dirty 
=========================

These are all the things. Use your browser's search to find things you want.

You'll need to ``#include <sol.hpp>``/``#include "sol.hpp"`` somewhere in your code. Sol is header-only, so you don't need to compile anything.

opening a state
---------------

.. code-block:: cpp
	
	int main (int argc, char* argv[]) {
		sol::state lua;
		// open some common libraries
		lua.open_libraries(sol::lib::base, sol::lib::package);
		lua.script( "print('bark bark bark!')" );
	}


sol::state on lua_State*
------------------------

For your system/game that already has lua, but you'd like something nice:

.. code-block:: cpp
	
	int pre_existing_system( lua_State* L ) {
		sol::state_view lua(L);
		lua.script( "print('bark bark bark!')" );
		return 0;
	}


running lua code
----------------

.. code-block:: cpp

	sol::state lua;
	// load and execute from string
	lua.script("a = 'test'");
	// load and execute from file
	lua.script_file("path/to/luascript.lua");

	// run a script, get the result
	int value = lua.script("return 54");
	// value == 54

	// load file without execute
	sol::load_result script1 = lua.load_file("path/to/luascript.lua");
	script1(); //execute
	// load string without execute
	sol::load_result script2 = lua.load("a = 'test'");
	script2(); //execute

	sol::load_result script3 = lua.load("return 24");
	int value2 = script3(); // execute, get return value
	// value2 == 24


set and get variables
---------------------

You can set/get everything.
	
.. code-block:: cpp
	
	sol::lua_state lua;

	lua.open_libraries(sol::lib::base);

	// integer types
	lua.set("number", 24);

	// floating point numbers
	lua["number2"] = 24.5;

	// string types
	lua["important_string"] = "woof woof";

	// non-recognized types is stored as userdata
	// is callable, therefore gets stored as a function
	lua["a_function"] = [](){ return 100; };

	// make a table
	lua["some_table"] = lua.create_table_wth("value", 24);


Equivalent to loading a lua file with:

.. code-block:: lua

	number = 24
	number2 = 24.5
	important_string = "woof woof"
	a_function = function () return 100 end
	some_table = { value = 24 }

Retrieve these variables using this syntax:

.. code-block:: cpp

	// implicit conversion
	int number = lua["number"];
	
	// explicit get
	auto number2 = lua.get<double>("number2");

	// strings too
	std::string important_string = lua["important_string"];

	// dig into a table
	int value = lua["value"]["value"];
	
	// get a function
	sol::function a_function = lua["a_function"];
	int value_is_100 = a_function();

	// get a std::function
	std::function<int()> a_std_function = lua["a_function"];
	int value_is_still_100 = a_std_function();

Retrieve Lua types using ``object`` and other ``sol::`` types.

.. code-block:: cpp

	sol::state lua;

	// ... everything from before

	sol::object number_obj = lua.get<sol::object>( "number" );
	// sol::type::number
	sol::type t1 = number_obj.get_type();

	sol::object function_obj = lua[ "a_function" ];
	// sol::type::function
	sol::type t2 = function_obj.get_type();
	bool is_it_really = function_obj.is<std::function<int()>(); // true

	// will not contain data
	sol::optional<int> check_for_me = lua["a_function"];


You can erase things by setting it to ``nullptr`` or ``sol::nil``.

.. code-block:: cpp

	sol::state lua;

	lua.script("exists = 250");

	int first_try = lua.get_or<int>( 322 );
	// first_try == 250

	lua.set("exists", sol::nil);

	int second_try = lua.get_or<int>( 322 );
	// second_try == 322


Note that if its a :doc:`userdata/usertype<../api/usertype>` for a C++ type, the destructor will run only when the garbage collector deems it appropriate to destroy the memory. If you are relying on the destructor being run when its set to ``sol::nil``, you're probably committing a mistake.

tables
------

:doc:`sol::state<../api/state>` is a table too.

.. code-block:: cpp

	sol::state lua;

	// Raw string literal for easy multiline
	lua.script( R"(
		abc = { [0] = 24 }
		def = { 
			ghi = { 
				bark = 50, 
				woof = abc 
			} 
		}
	)"
	);

	sol::table abc = lua["abc"];
	sol::table def = lua["def"];
	sol::table ghi = lua["def"]["ghi"];

	int bark1 = def["ghi"]["bark"];
	int bark2 = lua["def"]["ghi"]["bark"];
	// bark1 == bark2 == 50
	
	int abcval1 = abc[0];
	int abcval2 = ghi["woof"][0];
	// abcval1 == abcval2 == 24

If you're going deep, be safe:

.. code-block:: cpp

	sol::state lua;

	sol::optional<int> will_not_error = lua["abc"]["DOESNOTEXIST"]["ghi"];
	// will_not_error == sol::nullopt
	int will_not_error2 = lua["abc"]["def"]["ghi"]["jklm"].get_or<int>(25);
	// is 25

	// if you don't go safe,
	// will throw (or do at_panic if no exceptions)
	int aaaahhh = lua["abc"]["hope_u_liek_crash"];


make tables
-----------

Make some:

.. code-block:: cpp

	sol::state lua;

	lua["abc"] = lua.create_table_with(
		0, 24
	);

	lua.create_named_table("def",
		"ghi", lua.create_table_with(
			"bark", 50,
			// can reference other existing stuff too
			"woof", lua["abc"]
		)
	);

Equivalent Lua code:

.. code-block:: lua
	
	abc = { [0] = 24 }
	def = { 
		ghi = { 
			bark = 50, 
			woof = abc 
		} 
	}	
	

You can put anything you want in tables as values or keys, including strings, numbers, functions, other tables.

Note that this idea that things can be nested is important and will help later when you get into :ref:`namespacing<namespacing>`.


functions
---------

They're great. Use them:

.. code-block:: cpp
	
	sol::state lua;

	lua.script("function f (a, b, c, d) return 1 end");
	lua.script("function g (a, b) return a + b end");

	// fixed signature std::function<...>
	std::function<int(int, double, int, std::string)> stdfx = lua["f"];
	// sol::function is often easier: 
	// takes a variable number/types of arguments...
	sol::function fx = lua["f"];

	int is_one = stdfx(1, 34.5, 3, "bark");
	int is_also_one = fx(1, "boop", 3, "bark");

	// call through operator[]
	int is_three = lua["g"](1, 2);
	// is_three == 3
	double is_4_8 = lua["g"](2.4, 2.4);
	// is_4_8 == 4.8

If you need to protect against errors and parser problems and you're not ready to deal with Lua's `longjmp` problems (if you compiled with C), use :doc:`sol::protected_function<../api/protected_function>`.

You can bind member variables as functions too, as well as all KINDS of function-like things:

.. code-block:: cpp
	
	void some_function () {
		std::cout << "some function!" << std::endl;
	}

	void some_other_function () {
		std::cout << "some other function!" << std::endl;
	}

	struct some_class {
		int variable = 30;

		double member_function () {
			return 24.5;
		}
	};

	sol::state lua;
	lua.open_libraries(sol::lib::base);

	// put an instance of "some_class" into lua
	// (we'll go into more detail about this later
	// just know here that it works and is
	// put into lua as a userdata
	lua.set("sc", some_class());

	// binds a plain function
	lua["f1"] = some_function;
	lua.set_function("f2", &some_other_function);

	// binds just the member function
	lua["m1"] = &some_class::member_function;
	
	// binds the class to the type
	lua.set_function("m2", &some_class::member_function, some_class{});

	// binds just the member variable as a function
	lua["v1"] = &some_class::variable;
	
	// binds class with member variable as function
	lua.set_function("v2", &some_class::variable, some_class{});

The lua code to call these things is:

.. code-block:: lua	

	f1() -- some function!
	f2() -- some other function!
	
	-- need class instance if you don't bind it with the function
	print(m1(sc)) -- 24.5
	-- does not need class instance: was bound to lua with one 
	print(m2()) -- 24.5
	
	-- need class instance if you 
	-- don't bind it with the function
	print(v1(sc)) -- 30
	-- does not need class instance: 
	-- it was bound with one 
	print(v2()) -- 30

	-- can set, still 
	-- requires instance
	v1(sc, 212)
	-- can set, does not need 
	-- class instance: was bound with one 
	v2(254)

	print(v1(sc)) -- 212
	print(v2()) -- 254

Can use ``sol::readonly( &some_class::variable )`` to make a variable readonly and error if someone tries to write to it.


self call
---------

You can pass the 'self' argument through C++ to emulate 'member function' calls in Lua.

.. code-block:: cpp
	
	sol::state lua;

	lua.open_libraries(sol::lib::base, sol::lib::package, sol::lib::table);

	// a small script using 'self' syntax
	lua.script(R"(
	some_table = { some_val = 100 }

	function some_table:add_to_some_val(value)
	    self.some_val = self.some_val + value
	end

	function print_some_val()
	    print("some_table.some_val = " .. some_table.some_val)
	end
	)");

	// do some printing
	lua["print_some_val"]();
	// 100

	sol::table self = lua["some_table"];
	self["add_to_some_val"](self, 10);
	lua["print_some_val"]();



multiple returns from lua
-------------------------

.. code-block:: cpp
	
	sol::state lua;

	lua.script("function f (a, b, c) return a, b, c end");
	
	std::tuple<int, int, int> result;
	result = lua["f"](100, 200, 300); 
	// result == { 100, 200, 300 }
	int a, int b;
	std::string c;
	sol::tie( a, b, c ) = lua["f"](100, 200, "bark");
	// a == 100
	// b == 200
	// c == "bark"


multiple returns to lua
-----------------------

.. code-block:: cpp
	
	sol::state lua;

	lua["f"] = [](int a, int b, sol::object c) {
		// sol::object can be anything here: just pass it through
		return std::make_tuple( a, b, c );
	};
	
	std::tuple<int, int, int> result = lua["f"](100, 200, 300); 
	// result == { 100, 200, 300 }
	
	std::tuple<int, int, std::string> result2;
	result2 = lua["f"](100, 200, "BARK BARK BARK!")
	// result2 == { 100, 200, "BARK BARK BARK!" }

	int a, int b;
	std::string c;
	sol::tie( a, b, c ) = lua["f"](100, 200, "bark");
	// a == 100
	// b == 200
	// c == "bark"


C++ classes from C++
--------------------

Everything that is not a:

	* primitive type: ``bool``, ``char/short/int/long/long long``, ``float/double``
	* string type: ``std::string``, ``const char*``
	* function type: function pointers, ``lua_CFunction``, ``std::function``, :doc:`sol::function/sol::protected_function<../api/function>`, :doc:`sol::coroutine<../api/coroutine>`, member variable, member function
	* designated sol type: :doc:`sol::table<../api/table>`, :doc:`sol::thread<../api/thread>`, :doc:`sol::error<../api/error>`, :doc:`sol::object<../api/object>`
	* transparent argument type: :doc:`sol::variadic_arg<../api/variadic_args>`, :doc:`sol::this_state<../api/this_state>`
	* usertype<T> class: :doc:`sol::usertype<../api/usertype>`

Is set as a :doc:`userdata + usertype<../api/usertype>`.

.. code-block:: cpp

	struct Doge { 
		int tailwag = 50; 
	}

	Doge dog{};
	
	// Copy into lua: destroyed by Lua VM during garbage collection
	lua["dog"] = dog;
	// OR: move semantics - will call move constructor if present instead
	// Again, owned by Lua
	lua["dog"] = std::move( dog );
	lua["dog"] = Doge{};
	lua["dog"] = std::make_unique<Doge>();
	lua["dog"] = std::make_shared<Doge>();
	// Identical to above

	Doge dog2{};

	lua.set("dog", dog2);
	lua.set("dog", std::move(dog2));
	lua.set("dog", Doge{});
	lua.set("dog", std::unique_ptr<Doge>(new Doge()));
	lua.set("dog", std::shared_ptr<Doge>(new Doge()));

``std::unique_ptr``/``std::shared_ptr``'s reference counts / deleters will :doc:`be respected<../api/unique_usertype_traits>`. If you want it to refer to something, whose memory you know won't die in C++, do the following:

.. code-block:: cpp

	struct Doge { 
		int tailwag = 50; 
	}

	sol::state lua;
	lua.open_libraries(sol::lib::base);

	Doge dog{}; // Kept alive somehow

	// Later...
	// The following stores a reference, and does not copy/move
	// lifetime is same as dog in C++ 
	// (access after it is destroyed is bad)
	lua["dog"] = &dog;
	// Same as above: respects std::reference_wrapper
	lua["dog"] = std::ref(dog);
	// These two are identical to above
	lua.set( "dog", &dog );
	lua.set( "dog", std::ref( dog ) );

Get userdata in the same way as everything else:

.. code-block:: cpp

	struct Doge { 
		int tailwag = 50; 
	}

	sol::state lua;
	lua.open_libraries(sol::lib::base);

	Doge& dog = lua["dog"]; // References Lua memory
	Doge* dog_pointer = lua["dog"]; // References Lua memory
	Doge dog_copy = lua["dog"]; // Copies, will not affect lua

Note that you can change the data of usertype variables and it will affect things in lua if you get a pointer or a reference from Sol:

.. code-block:: cpp

	struct Doge { 
		int tailwag = 50; 
	}

	sol::state lua;
	lua.open_libraries(sol::lib::base);

	Doge& dog = lua["dog"]; // References Lua memory
	Doge* dog_pointer = lua["dog"]; // References Lua memory
	Doge dog_copy = lua["dog"]; // Copies, will not affect lua

	dog_copy.tailwag = 525;
	// Still 50
	lua.script("assert(dog.tailwag == 50)");

	dog.tailwag = 100;
	// Now 100
	lua.script("assert(dog.tailwag == 100)");


C++ classes put into Lua
------------------------

See this :doc:`section here<cxx-in-lua>` and after perhaps see if :doc:`simple usertypes suit your needs<../api/simple_usertype>`. Also check out some `a basic example`_, `special functions`_ and  `initializers`_, 


.. _namespacing:

namespacing
-----------

You can emulate namespacing by having a table and giving it the namespace names you want before registering enums or usertypes:

.. code-block:: cpp
	
	struct my_class {
		int b = 24;

		int f () const {
			return 24;
		}

		void g () {
			++b;
		}
	};

	sol::state lua;
	lua.open_libraries();

	// set up table
	sol::table bark = lua.create_named_table("bark");
	
	bark.new_usertype<my_class>( "my_class", 
		"f", &my_class::f,
		"g", &my_class::g
	); // the usual

	// 'bark' namespace
	lua.script("obj = bark.my_class.new()" );
	lua.script("obj:g()");
	my_class& obj = lua["obj"];
	// obj.b == 25


This technique can be used to register namespace-like functions and classes. It can be as deep as you want. Just make a table and name it appropriately, in either Lua script or using the equivalent Sol code. As long as the table FIRST exists (e.g., make it using a script or with one of Sol's methods or whatever you like), you can put anything you want specifically into that table using :doc:`sol::table's<../api/table>` abstractions.

advanced
--------

Some more advanced things you can do/read about:
	* :doc:`metatable manipulations<../api/metatable_key>` allow a user to change how indexing, function calls, and other things work on a single type.
	* :doc:`ownership semantics<ownership>` are described for how lua deals with (raw) pointers.
	* :doc:`stack manipulation<../api/stack>` to safely play with the stack. You can also define customization points for ``stack::get``/``stack::check``/``stack::push`` for your type.
	* :doc:`make_reference/make_object convenience function<../api/make_reference>` to get the same benefits and conveniences as the low-level stack API but put into objects you can specify.
	* :doc:`stack references<../api/stack_reference>` to have zero-overhead Sol abstractions while not copying to the Lua registry.
	* :doc:`unique usertype traits<../api/unique_usertype_traits>` allows you to specialize handle/RAII types from other frameworks, like boost, and Unreal, to work with Sol.
	* :doc:`variadic arguments<../api/variadic_args>` in functions with ``sol::variadic_args``.
	* :doc:`this_state<../api/this_state>` to get the current ``lua_State*``.
	* :doc:`resolve<../api/resolve>` overloads in case you have overloaded functions; a cleaner casting utility.

.. _a basic example: https://github.com/ThePhD/sol2/blob/develop/examples/usertype.cpp
.. _special functions: https://github.com/ThePhD/sol2/blob/develop/examples/usertype_special_functions.cpp
.. _initializers: https://github.com/ThePhD/sol2/blob/develop/examples/usertype_initializers.cpp

