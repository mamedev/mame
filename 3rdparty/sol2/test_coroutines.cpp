#define SOL_CHECK_ARGUMENTS

#include <catch.hpp>
#include <sol.hpp>

TEST_CASE("threading/coroutines", "ensure calling a coroutine works") {
	const auto& script = R"(counter = 20
 
function loop()
    while counter ~= 30
    do
        coroutine.yield(counter);
        counter = counter + 1;
    end
    return counter
end
)";

	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::coroutine);
	lua.script(script);
	sol::coroutine cr = lua["loop"];

	int counter;
	for (counter = 20; counter < 31 && cr; ++counter) {
		int value = cr();
		if (counter != value) {
			throw std::logic_error("fuck");
		}
	}
	counter -= 1;
	REQUIRE(counter == 30);
}

TEST_CASE("threading/new-thread-coroutines", "ensure calling a coroutine works when the work is put on a different thread") {
	const auto& script = R"(counter = 20
 
function loop()
    while counter ~= 30
    do
        coroutine.yield(counter);
        counter = counter + 1;
    end
    return counter
end
)";

	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::coroutine);
	lua.script(script);
	sol::thread runner = sol::thread::create(lua.lua_state());
	sol::state_view runnerstate = runner.state();
	sol::coroutine cr = runnerstate["loop"];

	int counter;
	for (counter = 20; counter < 31 && cr; ++counter) {
		int value = cr();
		if (counter != value) {
			throw std::logic_error("fuck");
		}
	}
	counter -= 1;
	REQUIRE(counter == 30);
}