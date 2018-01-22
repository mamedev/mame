#define SOL_CHECK_ARGUMENTS

#include <catch.hpp>
#include <sol.hpp>

TEST_CASE("storage/registry=construction", "ensure entries from the registry can be retrieved") {
	const auto& script = R"(
function f()
    return 2
end
)";

	sol::state lua;
	sol::function f = lua["f"];
	sol::reference r = lua["f"];
	sol::function regf(lua, f);
	sol::reference regr(lua, sol::ref_index(f.registry_index()));
	bool isequal = f == r;
	REQUIRE(isequal);
	isequal = f == regf;
	REQUIRE(isequal);
	isequal = f == regr;
	REQUIRE(isequal);
}

TEST_CASE("storage/main-thread", "ensure round-tripping and pulling out thread data even on 5.1 with a backup works") {
	sol::state lua;
	{
		sol::stack_guard g(lua);
		lua_State* orig = lua;
		lua_State* ts = sol::main_thread(lua, lua);
		REQUIRE(ts == orig);
	}
}
