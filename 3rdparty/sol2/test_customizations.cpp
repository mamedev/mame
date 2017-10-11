#define SOL_CHECK_ARGUMENTS

#include <catch.hpp>
#include <sol.hpp>

#include <unordered_map>
#include <vector>

struct two_things {
	int a;
	bool b;
};

namespace sol {

	// First, the expected size
	// Specialization of a struct
	template <>
	struct lua_size<two_things> : std::integral_constant<int, 2> {};

	// Now, specialize various stack structures
	namespace stack {

		template <>
		struct checker<two_things> {
			template <typename Handler>
			static bool check(lua_State* L, int index, Handler&& handler, record& tracking) {
				// Check first and second second index for being the proper types
				bool success = stack::check<int>(L, index, handler)
					&& stack::check<bool>(L, index + 1, handler);
				tracking.use(2);
				return success;
			}
		};

		template <>
		struct getter<two_things> {
			static two_things get(lua_State* L, int index, record& tracking) {
				// Get the first element
				int a = stack::get<int>(L, index);
				// Get the second element, 
				// in the +1 position from the first
				bool b = stack::get<bool>(L, index + 1);
				// we use 2 slots, each of the previous takes 1
				tracking.use(2);
				return two_things{ a, b };
			}
		};

		template <>
		struct pusher<two_things> {
			static int push(lua_State* L, const two_things& things) {
				int amount = stack::push(L, things.a);
				amount += stack::push(L, things.b);
				// Return 2 things
				return amount;
			}
		};

	}
}

TEST_CASE("customization/split-struct", "using the newly documented customization points to handle different kinds of classes") {
	sol::state lua;

	// Create a pass-through style of function
	lua.script("function f ( a, b, c ) return a + c, b end");
	lua.set_function("g", [](int a, bool b, int c, double d) {
		return std::make_tuple(a + c, b, d + 2.5);
	});

	// get the function out of Lua
	sol::function f = lua["f"];
	sol::function g = lua["g"];

	two_things thingsf = f(two_things{ 24, true }, 1);
	two_things thingsg;
	double d;
	sol::tie( thingsg, d ) = g(two_things{ 25, false }, 2, 34.0);
	REQUIRE(thingsf.a == 25);
	REQUIRE(thingsf.b);

	REQUIRE(thingsg.a == 27);
	REQUIRE_FALSE(thingsg.b);
	REQUIRE(d == 36.5);
}
