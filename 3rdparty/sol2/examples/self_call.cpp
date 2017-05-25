#define SOL_CHECK_ARGUMENTS
#include <sol.hpp>
#include <cassert>
#include <iostream>

int main() {
	std::cout << "=== self_call example ===" << std::endl;

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

	std::cout << std::endl;
}