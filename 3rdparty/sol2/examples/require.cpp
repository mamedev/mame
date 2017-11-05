#define SOL_CHECK_ARGUMENTS
#include <sol.hpp>
#include <cassert>

#include <iostream>

struct some_class {
	int bark = 2012;
};

sol::table open_mylib(sol::this_state s) {
	sol::state_view lua(s);

	sol::table module = lua.create_table();
	module["func"] = []() { /* super cool function here */ };
	// register a class too
	module.new_usertype<some_class>("some_class",
		"bark", &some_class::bark
		);

	return module;
}

int main() {
	std::cout << "=== require example ===" << std::endl;

	sol::state lua;
	lua.open_libraries(sol::lib::package);
	// sol::c_call takes functions at the template level
	// and turns it into a lua_CFunction
	// alternatively, one can use sol::c_call<sol::wrap<callable_struct, callable_struct{}>> to make the call
	// if it's a constexpr struct
	lua.require("my_lib", sol::c_call<decltype(&open_mylib), &open_mylib>);

	// do ALL THE THINGS YOU LIKE
	lua.script(R"(
s = my_lib.some_class.new()
s.bark = 20;
)");
	some_class& s = lua["s"];
	assert(s.bark == 20);
	std::cout << "s.bark = " << s.bark << std::endl;

	std::cout << std::endl;

	return 0;
}