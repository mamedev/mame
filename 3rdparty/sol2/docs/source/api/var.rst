var
===
For hooking up static / global variables to Lua usertypes
---------------------------------------------------------

The sole purpose of this tagging type is to work with :doc:`usertypes<usertype>` to provide ``my_class.my_static_var`` access, and to also provide reference-based access as well.

.. code-block:: cpp

	#include <sol.hpp>

	struct test {
		static int muh_variable;
	};
	int test::muh_variable = 25;
	

	int main () {
		sol::state lua;
		lua.open_libraries();
		lua.new_usertype<test>("test",
			"direct", sol::var(2),
			"global", sol::var(test::muh_variable),
			"ref_global", sol::var(std::ref(test::muh_variable))
		);

		int direct_value = lua["test"]["direct"];
		// direct_value == 2
		
		int global = lua["test"]["global"];
		// global == 25
		int global2 = lua["test"]["ref_global"];
		// global2 == 25

		test::muh_variable = 542;
		
		global = lua["test"]["global"];
		// global == 25
		// global is its own memory: was passed by value
		
		global2 = lua["test"]["ref_global"];
		// global2 == 542
		// global2 was passed through std::ref
		// global2 holds a reference to muh_variable
		// if muh_variable goes out of scope or is deleted
		// problems could arise, so be careful!

		return 0;
	}
