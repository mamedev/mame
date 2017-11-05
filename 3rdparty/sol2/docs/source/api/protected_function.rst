protected_function
==================
Lua function calls that trap errors and provide error handling
--------------------------------------------------------------

.. code-block:: cpp
	
	class protected_function : public reference;

Inspired by a request from `starwing<https://github.com/starwing>` in the old repository, this class provides the same interface as :doc:`function<function>` but with heavy protection and a potential error handler for any Lua errors and C++ exceptions. You can grab a function directly off the stack using the constructor, or pass to it 2 valid functions, which we'll demonstrate a little later.

When called without the return types being specified by either a ``sol::types<...>`` list or a ``call<Ret...>( ... )`` template type list, it generates a :doc:`protected_function_result<proxy>` class that gets implicitly converted to the requested return type. For example:

.. code-block:: lua
	:caption: pfunc_barks.lua
	:linenos:

	bark_power = 11;

	function got_problems( error_msg )
		return "got_problems handler: " .. error_msg
	end

	function woof ( bark_energy )
		if bark_energy < 20
			error("*whine*")
		end
		return (bark_energy * (bark_power / 4))
	end

	function woofers ( bark_energy )
		if bark_energy < 10
			error("*whine*")
		end
		return (bark_energy * (bark_power / 4))
	end

The following C++ code will call this function from this file and retrieve the return value, unless an error occurs, in which case you can bind an error handling function like so:

.. code-block:: cpp
	:linenos:

	sol::state lua;

	lua.open_file( "pfunc_barks.lua" );

	sol::protected_function problematicwoof = lua["woof"];
	problematicwoof.error_handler = lua["got_problems"];

	auto firstwoof = problematic_woof(20);
	if ( firstwoof.valid() ) {
		// Can work with contents
		double numwoof = first_woof;
	}
	else{
		// An error has occured
		sol::error err = first_woof;
	}

	// errors, calls handler and then returns a string error from Lua at the top of the stack
	auto secondwoof = problematic_woof(19);
	if (secondwoof.valid()) {
		// Call succeeded
		double numwoof = secondwoof;
	}
	else {
		// Call failed
		// Note that if the handler was successfully called, this will include
		// the additional appended error message information of
		// "got_problems handler: " ...
		sol::error err = secondwoof;
		std::string what = err.what();
	} 

This code is much more long-winded than its :doc:`function<function>` counterpart but allows a person to check for errors. The type here for ``auto`` are ``sol::protected_function_result``. They are implicitly convertible to result types, like all :doc:`proxy-style<proxy>` types are.

Alternatively, with a bad or good function call, you can use ``sol::optional`` to check if the call succeeded or failed:

.. code-block:: cpp
	:linenos:

	sol::state lua;

	lua.open_file( "pfunc_barks.lua" );

	sol::protected_function problematicwoof = lua["woof"];
	problematicwoof.error_handler = lua["got_problems"];

	sol::optional<double> maybevalue = problematicwoof(19);
	if (maybevalue) {
		// Have a value, use it
		double numwoof = maybevalue.value();
	}
	else {
		// No value!		
	}

That makes the code a bit more concise and easy to reason about if you don't want to bother with reading the error. Thankfully, unlike ``sol::function_result``, you can save ``sol::protected_function_result`` in a variable and push/pop things above it on the stack where its returned values are. This makes it a bit more flexible  than the rigid, performant ``sol::function_result`` type that comes from calling :doc:`sol::function<function>`.

If you're confident the result succeeded, you can also just put the type you want (like ``double`` or ``std::string`` right there and it will get it. But, if it doesn't work out, sol can throw and/or panic if you have the :doc:`safety<../safety>` features turned on:

.. code-block:: cpp
	:linenos:

	sol::state lua;

	lua.open_file( "pfunc_barks.lua" );

	// construct with function + error handler
	// shorter than old syntax
	sol::protected_function problematicwoof(lua["woof"], lua["got_problems"]);

	// dangerous if things go wrong!
	double value = problematicwoof(19);


Finally, it is *important* to note you can set a default handler. The function is described below: please use it to avoid having to constantly set error handlers:

.. code-block:: cpp
	:linenos:

	sol::state lua;

	lua.open_file( "pfunc_barks.lua" );
	// sets got_problems as the default
	// handler for all protected_function errors
	sol::protected_function::set_default_handler(lua["got_problems"]);

	sol::protected_function problematicwoof = lua["woof"];
	sol::protected_function problematicwoofers = lua["woofers"];

	double value = problematicwoof(19);
	double value2 = problematicwoof(9);


members
-------

.. code-block:: cpp
	:caption: constructor: protected_function

	template <typename T>
	protected_function( T&& func, reference handler = sol::protected_function::get_default_handler() );
	protected_function( lua_State* L, int index = -1, reference handler = sol::protected_function::get_default_handler() );

Constructs a ``protected_function``. Use the 2-argument version to pass a custom error handling function more easily. You can also set the :ref:`member variable error_handler<protected-function-error-handler>` after construction later. ``protected_function`` will always use the latest error handler set on the variable, which is either what you passed to it or the default *at the time of construction*. 

.. code-block:: cpp
	:caption: function: call operator / protected function call

	template<typename... Args>
	protected_function_result operator()( Args&&... args );

	template<typename... Ret, typename... Args>
	decltype(auto) call( Args&&... args );

	template<typename... Ret, typename... Args>
	decltype(auto) operator()( types<Ret...>, Args&&... args );

Calls the function. The second ``operator()`` lets you specify the templated return types using the ``my_func(sol::types<int, std::string>, ...)`` syntax. If you specify no return type in any way, it produces s ``protected_function_result``.

.. note::

	All arguments are forwarded. Unlike :doc:`get/set/operator[] on sol::state<state>` or :doc:`sol::table<table>`, value semantics are not used here. It is forwarding reference semantics, which do not copy/move unless it is specifically done by the receiving functions / specifically done by the user.


.. code-block:: cpp
	:caption: default handlers

	static const reference& get_default_handler ();
	static void set_default_handler( reference& ref );

Get and set the Lua entity that is used as the default error handler. The default is a no-ref error handler. You can change that by calling ``protected_function::set_default_handler( lua["my_handler"] );`` or similar: anything that produces a reference should be fine.

.. code-block:: cpp
	:caption: variable: handler
	:name: protected-function-error-handler

	reference error_handler;

The error-handler that is called should a runtime error that Lua can detect occurs. The error handler function needs to take a single string argument (use type std::string if you want to use a C++ function bound to lua as the error handler) and return a single string argument (again, return a std::string or string-alike argument from the C++ function if you're using one as the error handler). If :doc:`exceptions<../exceptions>` are enabled, Sol will attempt to convert the ``.what()`` argument of the exception into a string and then call the error handling function. It is a :doc:`reference<reference>`, as it must refer to something that exists in the lua registry or on the Lua stack. This is automatically set to the default error handler when ``protected_function`` is constructed.

.. note::

	``protected_function_result`` safely pops its values off the stack when its destructor is called, keeping track of the index and number of arguments that were supposed to be returned. If you remove items below it using ``lua_remove``, for example, it will not behave as expected. Please do not perform fundamentally stack-rearranging operations until the destructor is called (pushing/popping above it is just fine).

To know more about how function arguments are handled, see :ref:`this note<function-argument-handling>`.
