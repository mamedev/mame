#include <sol.hpp>

#include <iostream>
#include <cassert>

// Uses some of the fancier bits of sol2, including the "transparent argument",
// sol::this_state, which gets the current state and does not increment
// function arguments
sol::object fancy_func(sol::object a, sol::object b, sol::this_state s) {
	sol::state_view lua(s);
	if (a.is<int>() && b.is<int>()) {
		return sol::object(lua, sol::in_place, a.as<int>() + b.as<int>());
	}
	else if (a.is<bool>()) {
		bool do_triple = a.as<bool>();
		return sol::object(lua, sol::in_place<double>, b.as<double>() * (do_triple ? 3 : 1));
	}
	// Can also use make_object
	return sol::make_object(lua, sol::nil);
}

int main() {
	sol::state lua;

	lua["f"] = fancy_func;

	int result = lua["f"](1, 2);
	// result == 3
	assert(result == 3);
	double result2 = lua["f"](false, 2.5);
	// result2 == 2.5
	assert(result2 == 2.5);

	// call in Lua, get result
	// notice we only need 2 arguments here, not 3 (sol::this_state is transparent)
	lua.script("result3 = f(true, 5.5)");
	double result3 = lua["result3"];
	// result3 == 16.5
	assert(result3 == 16.5);
	
	std::cout << "=== any_return example ===" << std::endl;
	std::cout << "result : " << result << std::endl;
	std::cout << "result2: " << result2 << std::endl;
	std::cout << "result3: " << result3 << std::endl;
	std::cout << std::endl;
}