#include <string>
#include "sol.hpp"

#include <iostream>

int main() {
	std::cout << "=== coroutine example ===" << std::endl;

	sol::state lua;
	std::vector<sol::coroutine> tasks;

	lua.open_libraries(sol::lib::base, sol::lib::coroutine);

	sol::thread runner_thread = sol::thread::create(lua);

	lua.set_function("start_task",
		[&runner_thread, &tasks](sol::function f, sol::variadic_args va) {
		// You must ALWAYS get the current state
		sol::state_view runner_thread_state = runner_thread.state();
		// Put the task in our task list to keep it alive and track it
		std::size_t task_index = tasks.size();
		tasks.emplace_back(runner_thread_state, f);
		sol::coroutine& f_on_runner_thread = tasks[task_index];
		// call coroutine with arguments that came 
		// from main thread / other thread
		// pusher for `variadic_args` and other sol types will transfer the
		// arguments from the calling thread to 
		// the runner thread automatically for you
		// using `lua_xmove` internally
		int wait = f_on_runner_thread(va);
		std::cout << "First return: " << wait << std::endl;
		// When you call it again, you don't need new arguments 
		// (they remain the same from the first call)
		f_on_runner_thread();
		std::cout << "Second run complete: " << wait << std::endl;
	});

	lua.script(
		R"(
function main(x, y, z)
	-- do something
	coroutine.yield(20)
	-- do something else
	-- do ...
	print(x, y, z)
end

function main2(x, y)
	coroutine.yield(10)
	print(x, y)
end

	start_task(main, 10, 12, 8)
	start_task(main2, 1, 2)
)"
);

	return 0;
}
