thread
======
a separate state that can contain and run functions
---------------------------------------------------

.. code-block:: cpp
	
	class thread : public reference { /* ... */ };

``sol::thread`` is a separate runnable part of the Lua VM that can be used to execute work separately from the main thread, such as with :doc:`coroutines<coroutine>`. To take a table or a coroutine and run it specifically on the ``sol::thread`` you either pulled out of lua or created, just get that function through the :ref:`state of the thread<thread_state>`

.. note::

	A CPU thread is not always equivalent to a new thread in Lua: ``std::this_thread::get_id()`` can be the same for 2 callbacks that have 2 distinct Lua threads. In order to know which thread a callback was called in, hook into :doc:`sol::this_state<this_state>` from your Lua callback and then construct a ``sol::thread``, passing in the ``sol::this_state`` for both the first and last arguments. Then examine the results of the status and ``is_...`` calls below.

free function
-------------

.. code-block:: cpp
	:caption: function: main_thread

	main_thread(lua_State* current, lua_State* backup_if_bad_platform = nullptr);

The function ``sol::main_thread( ... )`` retrieves the main thread of the application on Lua 5.2 and above *only*. It is designed for code that needs to be multithreading-aware (e.g., uses multiple :doc:`threads<thread>` and :doc:`coroutines<coroutine>`).

.. warning::
	
	This code function will be present in Lua 5.1/LuaJIT, but only have proper behavior when given a single argument on Lua 5.2 and beyond. Lua 5.1 does not support retrieving the main thread from its registry, and therefore it is entirely suggested if you are writing cross-platform Lua code that you must store the main thread of your application in some global storage accessible somewhere. Then, pass this item into the ``sol::main_thread( possibly_thread_state, my_actual_main_state )`` and it will select that ``my_actual_main_state`` every time. If you are not going to use Lua 5.1 / LuaJIT, you can ignore the last parameter.


members
-------

.. code-block:: cpp
	:caption: constructor: thread

	thread(stack_reference r);
	thread(lua_State* L, int index = -1);
	thread(lua_State* L, lua_State* actual_thread);

Takes a thread from the Lua stack at the specified index and allows a person to use all of the abstractions therein. It can also take an actual thread state to make a thread from that as well.

.. code-block:: cpp
	:caption: function: view into thread_state()'s state

	state_view state() const;

This retrieves the current state of the thread, producing a :doc:`state_view<state>` that can be manipulated like any other. :doc:`Coroutines<coroutine>` pulled from Lua using the thread's state will be run on that thread specifically.

.. _thread_state:

.. code-block:: cpp
	:caption: function: retrieve thread state object

	lua_State* thread_state () const;

This function retrieves the ``lua_State*`` that represents the thread.

.. code-block:: cpp
	:caption: current thread status

	thread_status status () const;

Retrieves the :doc:`thread status<types>` that describes the current state of the thread.

.. code-block:: cpp
	:caption: main thread status

	bool is_main_thread () const;

Checks to see if the thread is the main Lua thread.

.. code-block:: cpp
	:caption: function: thread creation
	:name: thread-create

	thread create();
	static thread create (lua_State* L);

Creates a new thread from the given a ``lua_State*``.