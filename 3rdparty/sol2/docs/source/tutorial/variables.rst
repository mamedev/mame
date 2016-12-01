variables
=========

Working with variables is easy with Sol, and behaves pretty much like any associative array / map structure you've dealt with previously. Given this lua file that gets loaded into Sol:

reading
-------

.. code-block:: lua
	:caption: variables.lua

	config = {
		fullscreen = false,
		resolution = { x = 1024, y = 768 }
	}

.. code-block:: cpp
	:caption: main.cpp
	:name: variables-main-cpp

	#include <sol.hpp>

	int main () {

		sol::state lua;
		lua.script_file( variables.lua );

		return 0;
	}

You can interact with the variables like this:

.. code-block:: cpp
	:caption: main.cpp extended
	:name: extended-variables-main-cpp

	#include <sol.hpp>
	#include <tuple>
	#include <utility> // for std::pair

	int main () {

		sol::state lua;
		lua.script_file( variables.lua );

		// the type "state" behaves exactly like a table!
		bool isfullscreen = lua["config"]["fullscreen"]; // can get nested variables
		sol::table config = lua["config"];
		
		// can also get it using the "get" member function
		// auto replaces the unqualified type name
		auto resolution = config.get<sol::table>( "config" );

		// table and state can have multiple things pulled out of it too
		std::pair<int, int> xyresolution = resolution.get<int, int>( "x", "y" );
		// As an example, you can also pull out a tuple as well
		std::tuple<int, int> xyresolutiontuple = resolution.get<int, int>( "x", "y" );


		return 0;
	}

From this example, you can see that there's many ways to pull out the varaibles you want. You can get  For example, to determine if a nested variable exists or not, you can use ``auto`` to capture the value of a ``table[key]`` lookup, and then use the ``.valid()`` method:

.. code-block:: cpp
	:caption: safe lookup

	auto bark = lua["config"]["bark"];
	if (bark.valid()) {
		// branch not taken: config / bark is not a variable
	}
	else {
		// Branch taken: config is a not a variable
	}

This comes in handy when you want to check if a nested variable exists. You can also check if a toplevel variable is present or not by using ``sol::optional``, which also checks if A) the keys you're going into exist and B) the type you're trying to get is of a specific type:

.. code-block:: cpp
	:caption: optional lookup

	sol::optional<int> not_an_integer = lua["config"]["fullscreen"];
	if (not_an_integer) {
		// Branch not taken: value is not an integer
	}

	sol::optoinal<bool> is_a_boolean = lua["config"]["fullscreen"];
	if (is_a_boolean) {
		// Branch taken: the value is a boolean
	}

	sol::optional<double> does_not_exist = lua["not_a_variable"];
	if (does_not_exist) {
		// Branch not taken: that variable is not present
	}

This can come in handy when, even in optimized or release modes, you still want the safety of checking.  You can also use the `get_or` methods to, if a certain value may be present but you just want to default the value to something else:

.. code-block:: cpp
	:caption: get_or lookup

	// this will result in a value of '24'
	int is_defaulted = lua["config"]["fullscreen"].get_or( 24 );

	// This will result in the value of the config, which is 'false'
	bool is_not_defaulted = lua["config"]["fullscreen"];

That's all it takes to read variables!


writing
-------

Writing gets a lot simpler. Even without scripting a file or a string, you can read and write variables into lua as you please:

.. code-block:: cpp
	:caption: main.cpp
	:name: writing-main-cpp

	#include <sol.hpp>
	#include <iostream>

	int main () {

		sol::state lua;

		// open those basic lua libraries again, like print() etc.
		lua.open_libraries( sol::lib::base );

		// value in the global table
		lua["bark"] = 50;

		// a table being created in the global table
		lua["some_table"] = lua.create_table_with(
			"key0", 24, 
			"key1", 25,
			lua["bark"], "the key is 50 and this string is its value!"
		);

		// Run a plain ol' string of lua code
		// Note you can interact with things set through Sol in C++ with lua!
		// Using a "Raw String Literal" to have multi-line goodness: http://en.cppreference.com/w/cpp/language/string_literal
		lua.script(R"(
		
		print(some_table[50])
		print(some_table["key0"])
		print(some_table["key1"])

		-- a lua comment: access a global in a lua script with the _G table
		print(_G["bark"])

		)");

		return 0;
	}

This example pretty much sums up what can be done. Note that the syntax ``lua["non_existing_key_1"] = 1`` will make that variable, but if you tunnel too deep without first creating a table, the Lua API will panic (e.g., ``lua["does_not_exist"]["b"] = 20`` will trigger a panic). You can also be lazy with reading / writing values:

.. code-block:: cpp
	:caption: main.cpp
	:name: lazy-main-cpp

	#include <sol.hpp>
	#include <iostream>

	int main () {

		sol::state lua;

		auto barkkey = lua["bark"];
		if (barkkey.valid()) {
			// Branch not taken: doesn't exist yet
			std::cout << "How did you get in here, arf?!" << std::endl;
		}

		barkkey = 50;
		if (barkkey.valid()) {
			// Branch taken: value exists!
			std::cout << "Bark Bjork Wan Wan Wan" << std::endl;
		}
	}

Finally, it's possible to erase a reference/variable by setting it to ``nil``, using the constant ``sol::nil`` in C++:

.. code-block:: cpp
	:caption: main.cpp
	:name: erase-main-cpp

	#include <sol.hpp>

	int main () {

		sol::state lua;
		lua["bark"] = 50;
		sol::optional<int> x = lua["bark"];
		// x will have a value

		lua["bark"] = sol::nil;
		sol::optional<int> y = lua["bark"];
		// y will not have a value
	}

It's easy to see that there's a lot of options to do what you want here. But, these are just traditional numbers and strings. What if we want more power, more capabilities than what these limited types can offer us? Let's throw some :doc:`functions in there<functions>` :doc:`C++ classes into the mix<cxx-in-lua>`!