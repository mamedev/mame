protect
=======
Routine to mark a function / variable as requiring safety
---------------------------------------------------------

.. code-block:: cpp
	
	template <typename T>
	auto protect( T&& value );

``sol::protect( my_func )`` allows you to protect a function call or member variable call when it is being set to Lua. It can be used with usertypes or when just setting a function into Sol. Below is an example that demonstrates that a call that would normally not error without :doc:`Safety features turned on<../safety>` that instead errors and makes the Lua safety-call wrapper ``pcall`` fail:

.. code-block:: cpp

	struct protect_me {
		int gen(int x) {
			return x;
		}
	};

	sol::state lua;
	lua.open_libraries(sol::lib::base);
	lua.new_usertype<protect_me>("protect_me", 
		"gen", sol::protect( &protect_me::gen )
	);

	lua.script(R"__(
	pm = protect_me.new()
	value = pcall(pm.gen,pm)
	)__");
	);
	bool value = lua["value"];
	// value == false
