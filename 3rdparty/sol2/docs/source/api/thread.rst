thread
======
a separate state that can contain and run functions
---------------------------------------------------

.. code-block:: cpp
	
	class thread : public reference { /* ... */ };

``sol::thread`` is a separate runnable part of the Lua VM that can be used to execute work separately from the main thread, such as with :doc:`coroutines<coroutine>`. To take a table or a coroutine and run it specifically on the ``sol::thread`` you either pulled out of lua or created, just get that function through the :ref:`state of the thread<thread_state>`

members
-------

.. code-block:: cpp
	:caption: constructor: thread

	thread(lua_State* L, int index = -1);

Takes a thread from the Lua stack at the specified index and allows a person to use all of the abstractions therein.

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
	:caption: function: thread creation
	:name: thread-create

	thread create();
	static thread create (lua_State* L);

Creates a new thread from the given a ``lua_State*``.