#define SOL_CHECK_ARGUMENTS
#include <sol.hpp>
#include <iostream>
#include <cassert>
#include <cmath>

struct vec {
	double x;
	double y;

	vec() : x(0), y(0) {}
	vec(double x, double y) : x(x), y(y) {}

	vec operator-(const vec& right) const {
		return vec(x - right.x, y - right.y);
	}
};

double dot(const vec& left, const vec& right) {
	return left.x * right.x + left.x * right.x;
}

vec operator+(const vec& left, const vec& right) {
	return vec(left.x + right.x, left.y + right.y);
}

int main() {
	sol::state lua;
	lua.open_libraries();

	lua.new_usertype<vec>("vec", 
		sol::constructors<sol::types<>, sol::types<double, double>>(),
		"dot", &dot,
		"norm", [](const vec& self) { double len = std::sqrt(dot(self, self)); return vec(self.x / len, self.y / len); },
		// we use `sol::resolve` because other operator+ can exist
		// in the (global) namespace
		sol::meta_function::addition, sol::resolve<const vec&, const vec&>(::operator+),
		sol::meta_function::subtraction, &vec::operator-
	);

	lua.script(R"(
		v1 = vec.new(1, 0)
		v2 = vec.new(0, 1)
		-- as "member function"
		d1 = v1:dot(v2)
		-- as "static" / "free function"
		d2 = vec.dot(v1, v2)
		assert(d1 == d2)

		-- doesn't matter if
		-- bound as free function
		-- or member function:
		a1 = v1 + v2
		s1 = v1 - v2
)");

	vec& a1 = lua["a1"];
	vec& s1 = lua["s1"];

	assert(a1.x == 1 && a1.y == 1);
	assert(s1.x == 1 && s1.y == -1);

	lua["a2"] = lua["a1"];

	lua.script(R"(
		assert(a1 == a2)
	)");

	std::cout << std::endl;
}