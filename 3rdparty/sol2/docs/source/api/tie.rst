tie
===
An improved version of ``std::tie``
-----------------------------------

`std::tie()`_ does not work well with :doc:`sol::function<function>`'s ``sol::function_result`` returns. Use ``sol::tie`` instead. Because they're both named `tie`, you'll need to be explicit when you use Sol's by naming it with the namespace (``sol::tie``), even with a ``using namespace sol;``. Here's an example:

.. code-block:: cpp

	sol::state lua;
	lua.open_libraries(sol::lib::base);

	const auto& code = R"(
	function test()
    		return 1, 2, 3
	end
	)";
	lua.script(code);
	
	int a, b, c;
	//std::tie(a, b, c) = lua["test"]();
	// will error: use the line below
	sol::tie(a, b, c) = lua["test"]();
	

.. _std::tie(): http://en.cppreference.com/w/cpp/utility/tuple/tie
