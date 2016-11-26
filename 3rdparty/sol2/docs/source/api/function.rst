function
========
calling functions bound to Lua
------------------------------

.. note::

	This abstraction assumes the function runs safely. If you expect your code to have errors (e.g., you don't always have explicit control over it or are trying to debug errors), please use :doc:`sol::protected_function<protected_function>`.

.. code-block:: cpp
	
	class function : public reference;

Function is a correct-assuming version of :doc:`protected_function<protected_function>`, omitting the need for typechecks and error handling. It is the default function type of Sol. Grab a function directly off the stack using the constructor:

.. code-block:: cpp
	:caption: constructor: function

	function(lua_State* L, int index = -1);


When called without the return types being specified by either a ``sol::types<...>`` list or a ``call<Ret...>( ... )`` template type list, it generates a :ref:`function_result<function-result>` class that gets implicitly converted to the requested return type. For example:

.. code-block:: lua
	:caption: func_barks.lua
	:linenos:

	bark_power = 11;

	function woof ( bark_energy )
		return (bark_energy * (bark_power / 4))
	end

The following C++ code will call this function from this file and retrieve the return value:

.. code-block:: cpp
	:linenos:

	sol::state lua;

	lua.script_file( "func_barks.lua" );

	sol::function woof = lua["woof"];
	double numwoof = woof(20);

The call ``woof(20)`` generates a :ref:`function_result<function-result>`, which is then implicitly converted to an ``double`` after being called. The intermediate temporary ``function_result`` is then destructed, popping the Lua function call results off the Lua stack. 

You can also return multiple values by using ``std::tuple``, or if you need to bind them to pre-existing variables use ``sol::tie``:

.. code-block:: cpp
	:linenos:

	sol::state lua;

	lua.script( "function f () return 10, 11, 12 end" );

	sol::function f = lua["f"];
	std::tuple<int, int, int> abc = f(); // 10, 11, 12 from Lua
	// or
	int a, b, c;
	sol::tie(a, b, c) = f(); // a = 10, b = 11, c = 12 from Lua

This makes it much easier to work with multiple return values. Using ``std::tie`` from the C++ standard will result in dangling references or bad behavior because of the very poor way in which C++ tuples/``std::tie`` were specified and implemented: please use ``sol::tie( ... )`` instead to satisfy any multi-return needs.

.. _function-result-warning:

.. warning::

	Do NOT save the return type of a :ref:`function_result<function-result>` with ``auto``, as in ``auto numwoof = woof(20);``, and do NOT store it anywhere. Unlike its counterpart :ref:`protected_function_result<protected-function-result>`, ``function_result`` is NOT safe to store as it assumes that its return types are still at the top of the stack and when its destructor is called will pop the number of results the function was supposed to return off the top of the stack. If you mess with the Lua stack between saving ``function_result`` and it being destructed, you will be subject to an incredible number of surprising and hard-to-track bugs. Don't do it.

.. code-block:: cpp
	:caption: function: call operator / function call

	template<typename... Args>
	protected_function_result operator()( Args&&... args );

	template<typename... Ret, typename... Args>
	decltype(auto) call( Args&&... args );

	template<typename... Ret, typename... Args>
	decltype(auto) operator()( types<Ret...>, Args&&... args );

Calls the function. The second ``operator()`` lets you specify the templated return types using the ``my_func(sol::types<int, std::string>, ...)`` syntax. Function assumes there are no runtime errors, and thusly will call the ``atpanic`` function if an error does occur.


.. _function-argument-handling:

functions and argument passing
------------------------------

.. note::

	All arguments are forwarded. Unlike :doc:`get/set/operator[] on sol::state<state>` or :doc:`sol::table<table>`, value semantics are not used here. It is forwarding reference semantics, which do not copy/move unless it is specifically done by the receiving functions / specifically done by the user.


.. note::

	This also means that you should pass and receive arguments in certain ways to maximize efficiency. For example, ``sol::table``, ``sol::object``, ``sol::userdata`` and friends are fairly cheap to copy, and should simply by taken as values. This includes primitive types like ``int`` and ``double``. However, C++ types -- if you do not want copies -- should be taken as ``const type&`` or ``type&``, to save on copies if it's important. Note that taking references from Lua also means you can modify the data inside of Lua directly, so be careful. Lua by default deals with things mostly by reference (save for primitive types).

	You can get even more speed out of ``sol::object`` style of types by taking a ``sol::stack_object`` (or ``sol::stack_...``, where ``...`` is ``userdata``, ``reference``, ``table``, etc.). These reference a stack position directly rather than cheaply/safely the internal Lua reference to make sure it can't be swept out from under you. Note that if you manipulate the stack out from under these objects, they may misbehave, so please do not blow up your Lua stack when working with these types.

	``std::string`` (and ``std::wstring``) are special. Lua stores strings as ``const char*`` null-terminated strings. ``std::string`` will copy, so taking a ``std::string`` by value or by const reference still invokes a copy operation. You can take a ``const char*``, but that will mean you're exposed to what happens on the Lua stack (if you change it and start chopping off function arguments from it in your function calls and such, as warned about previously).


function call safety
--------------------

You can have functions here and on usertypes check to definitely make sure that the types passed to C++ functions are what they're supposed to be by adding a ``#define SOL_CHECK_ARGUMENTS`` before including Sol, or passing it on the command line. Otherwise, for speed reasons, these checks are only used where absolutely necessary (like discriminating between :doc:`overloads<overload>`). See :doc:`safety<../safety>` for more information.