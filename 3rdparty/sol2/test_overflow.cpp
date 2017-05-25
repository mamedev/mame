#define SOL_CHECK_ARGUMENTS

#include <catch.hpp>
#include <sol.hpp>


TEST_CASE("issues/stack-overflow", "make sure various operations repeated don't trigger stack overflow") {
	sol::state lua;
	lua.script("t = {};t[0]=20");
	lua.script("lua_function=function(i)return i;end");

	sol::function f = lua["lua_function"];
	std::string teststring = "testtext";
	REQUIRE_NOTHROW(
		for (int i = 0; i < 1000000; ++i) {
			std::string result = f(teststring);
			if (result != teststring) throw std::logic_error("RIP");
		}
	);
	sol::table t = lua["t"];
	int expected = 20;
	REQUIRE_NOTHROW(
		for (int i = 0; i < 1000000; ++i) {
			int result = t[0];
			t.size();
			if (result != expected)
				throw std::logic_error("RIP");
		}
	);
}


TEST_CASE("issues/stack-overflow-2", "make sure basic iterators clean up properly when they're not iterated through (e.g., with empty())") {
	sol::state lua;
	sol::table t = lua.create_table_with(1, "wut");
	int MAX = 50000;
	auto fx = [&]() {
		int a = 50;
		for (int i = 0; i < MAX; ++i) {
			if (t.empty()) {
				a += 4;
			}
			a += 2;
		}
	};
	REQUIRE_NOTHROW(fx());
}
