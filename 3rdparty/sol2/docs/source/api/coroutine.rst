coroutine
=========
resumable/yielding functions from Lua
-------------------------------------

A ``coroutine`` is a :doc:`reference<reference>` to a function in Lua that can be called multiple times to yield a specific result. It is run on the :doc:`lua_State<state>` that was used to create it (see :doc:`thread<thread>` for an example on how to get a coroutine that runs on a thread separate from your usual "main" :doc:`lua_State<state>`).

The ``coroutine`` object is entirely similar to the :doc:`protected_function<protected_function>` object, with additional member functions to check if a coroutine has yielded (:doc:`call_status::yielded<types>`) and is thus runnable again, whether it has completed (:ref:`call_status::ok<call-status>`) and thus cannot yield anymore values, or whether it has suffered an error (see :ref:`status()<status>` and :ref:`call_status<call-status>`'s error codes).

For example, you can work with a coroutine like this:

.. code-block:: lua
    :caption: co.lua

        function loop()
            while counter ~= 30
            do
                coroutine.yield(counter);
                counter = counter + 1;
            end
            return counter
        end

This is a function that yields:

.. code-block:: cpp
    :caption: main.cpp

    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::coroutine);
    lua.script_file("co.lua");
    sol::coroutine cr = lua["loop"];

    for (int counter = 0; // start from 0 
        counter < 10 && cr; // we want 10 values, and we only want to run if the coroutine "cr" is valid
        // Alternative: counter < 10 && cr.valid()
        ++counter) {
            // Call the coroutine, does the computation and then suspends
            int value = cr();
    }

Note that this code doesn't check for errors: to do so, you can call the function and assign it as ``auto result = cr();``, then check ``result.valid()`` as is the case with :doc:`protected_function<protected_function>`. Finally, you can  run this coroutine on another thread by doing the following:

.. code-block:: cpp
    :caption: main_with_thread.cpp

    sol::state lua;
    lua.open_libraries(sol::lib::base, sol::lib::coroutine);
    lua.script_file("co.lua");
    sol::thread runner = sol::thread::create(lua.lua_state());
    sol::state_view runnerstate = runner.state();
    sol::coroutine cr = runnerstate["loop"];

    for (int counter = 0; counter < 10 && cr; ++counter) {
        // Call the coroutine, does the computation and then suspends
        int value = cr();
    }

The following are the members of ``sol::coroutine``:

members
-------

.. code-block:: cpp
    :caption: function: constructor

    coroutine(lua_State* L, int index = -1);

Grabs the coroutine at the specified index given a ``lua_State*``. 

.. code-block:: cpp
	:caption: returning the coroutine's status
	:name: status

	call_status status() const noexcept;

Returns the status of a coroutine.


.. code-block:: cpp
	:caption: checks for an error

	bool error() const noexcept;

Checks if an error occured when the coroutine was run.

.. _runnable:

.. code-block:: cpp
	:caption: runnable and explicit operator bool

	bool runnable () const noexcept;
	explicit operator bool() const noexcept;

These functions allow you to check if a coroutine can still be called (has more values to yield and has not errored). If you have a coroutine object ``coroutine my_co = /*...*/``, you can either check ``runnable()`` or do ``if ( my_co ) { /* use coroutine */ }``.

.. code-block:: cpp
	:caption: calling a coroutine

	template<typename... Args>
	protected_function_result operator()( Args&&... args );

	template<typename... Ret, typename... Args>
	decltype(auto) call( Args&&... args );

	template<typename... Ret, typename... Args>
	decltype(auto) operator()( types<Ret...>, Args&&... args );

Calls the coroutine. The second ``operator()`` lets you specify the templated return types using the ``my_co(sol::types<int, std::string>, ...)`` syntax. Check ``status()`` afterwards for more information about the success of the run or just check the coroutine object in an ifs tatement, as shown :ref:`above<runnable>`.