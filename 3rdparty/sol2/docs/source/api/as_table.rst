as_table
===========
make sure an object is pushed as a table
----------------------------------------

.. code-block:: cpp
	
	template <typename T>
	as_table_t { ... };

	template <typename T>
	as_table_t<T> as_function ( T&& container );

This function serves the purpose of ensuring that an object is pushed -- if possible -- like a table into Lua. The container passed here can be a pointer, a reference, a ``std::reference_wrapper`` around a container, or just a plain container value. It must have a begin/end function, and if it has a ``std::pair<Key, Value>`` as its ``value_type``, it will be pushed as a dictionary. Otherwise, it's pushed as a sequence.

.. code-block:: cpp

	sol::state lua;
	lua.open_libraries();
	lua.set("my_table", sol::as_table(std::vector<int>{ 1, 2, 3, 4, 5 }));
	lua.script("for k, v in ipairs(my_table) do print(k, v) assert(k == v) end");
	

Note that any caveats with Lua tables apply the moment it is serialized, and the data cannot be gotten out back out in C++ as a C++ type without explicitly using the ``as_table_t`` marker for your get and conversion operations using Sol.

If you need this functionality with a member variable, use a :doc:`property on a getter function<property>` that returns the result of ``sol::as_table``.

This marker does NOT apply to :doc:`usertypes<usertype>`.