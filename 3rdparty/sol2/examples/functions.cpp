#include <sol.hpp>
#include <cassert>
#include <iostream>

inline int my_add(int x, int y) {
    return x + y;
}

struct multiplier {
    int operator()(int x) {
        return x * 10;
    }

    static int by_five(int x) {
        return x * 5;
    }
};

inline std::string make_string( std::string input ) { 
	return "string: " + input;
}

int main() {
	std::cout << "=== functions example ===" << std::endl;

	sol::state lua;
	lua.open_libraries(sol::lib::base);

	// setting a function is simple
	lua.set_function("my_add", my_add);

	// you could even use a lambda
	lua.set_function("my_mul", [](double x, double y) { return x * y; });

	// member function pointers and functors as well
	lua.set_function("mult_by_ten", multiplier{});
	lua.set_function("mult_by_five", &multiplier::by_five);

	// assert that the functions work
	lua.script("assert(my_add(10, 11) == 21)");
	lua.script("assert(my_mul(4.5, 10) == 45)");
	lua.script("assert(mult_by_ten(50) == 500)");
	lua.script("assert(mult_by_five(10) == 50)");

	// using lambdas, functions can have state.
	int x = 0;
	lua.set_function("inc", [&x]() { x += 10; });

	// calling a stateful lambda modifies the value
	lua.script("inc()");
	assert(x == 10);
	if (x == 10) {
		// Do something based on this information
		std::cout << "Yahoo! x is " << x << std::endl;
	}

	// this can be done as many times as you want
	lua.script(R"(
inc()
inc()
inc()
)");
	assert(x == 40);
	if (x == 40) {
		// Do something based on this information
		std::cout << "Yahoo! x is " << x << std::endl;
	}
	// retrieval of a function is done similarly
	// to other variables, using sol::function
	sol::function add = lua["my_add"];
	int value = add(10, 11);
	// second way to call the function
	int value2 = add.call<int>(10, 11);
	assert(value == 21);
	assert(value2 == 21);
	if (value == 21 && value2 == 21) {
		std::cout << "Woo, value is 21!" << std::endl;
	}

	// multi-return functions are supported using
	// std::tuple as the interface.
	lua.set_function("multi", [] { return std::make_tuple(10, "goodbye"); });
	lua.script("x, y = multi()");
	lua.script("assert(x == 10 and y == 'goodbye')");

	auto multi = lua.get<sol::function>("multi");
	int first;
	std::string second;
	sol::tie(first, second) = multi();

	// use the values
	assert(first == 10);
	assert(second == "goodbye");

	// you can even overload functions
	// just pass in the different functions
	// you want to pack into a single name:
	// make SURE they take different types!
	
	lua.set_function("func", sol::overload([](int x) { return x; }, make_string, my_add));

	// All these functions are now overloaded through "func"
	lua.script(R"(
print(func(1))
print(func("bark"))
print(func(1,2))
)");

	std::cout << std::endl;
}