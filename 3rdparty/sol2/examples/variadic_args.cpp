#include <sol.hpp>

#include <iostream>

int main() {
	std::cout << "=== variadic_args example ===" << std::endl;

	sol::state lua;
	lua.open_libraries(sol::lib::base);

	// Function requires 2 arguments
	// rest can be variadic, but:
	// va will include everything after "a" argument,
	// which means "b" will be part of the varaidic_args list too
	// at position 0
	lua.set_function("v", [](int a, sol::variadic_args va, int /*b*/) {
		int r = 0;
		for (auto v : va) {
			int value = v; // get argument out (implicit conversion)
						   // can also do int v = va.get<int>(i); with index i
			r += value;
		}
		// Only have to add a, b was included from variadic_args and beyond
		return r + a;
	});

	lua.script("x = v(25, 25)");
	lua.script("x2 = v(25, 25, 100, 50, 250, 150)");
	lua.script("x3 = v(1, 2, 3, 4, 5, 6)");
	// will error: not enough arguments
	//lua.script("x4 = v(1)");

	lua.script("assert(x == 50)");
	lua.script("assert(x2 == 600)");
	lua.script("assert(x3 == 21)");
	lua.script("print(x)"); // 50
	lua.script("print(x2)"); // 600
	lua.script("print(x3)"); // 21
	std::cout << std::endl;
}