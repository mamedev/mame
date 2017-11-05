#define SOL_CHECK_ARGUMENTS
#include <sol.hpp>
#include <cassert>

int main() {
	struct my_class {
		int b = 24;

		int f() const {
			return 24;
		}

		void g() {
			++b;
		}
	};

	sol::state lua;
	lua.open_libraries();

	sol::table bark = lua.create_named_table("bark");
	bark.new_usertype<my_class>("my_class",
		"f", &my_class::f,
		"g", &my_class::g
		); // the usual

	lua.script("obj = bark.my_class.new()"); // this works
	lua.script("obj:g()");
	my_class& obj = lua["obj"];
	assert(obj.b == 25);

	return 0;
}
