light<T>/user<T>
================
Utility class for the cheapest form of (light) userdata
-------------------------------------------------------

.. code-block:: cpp
	
	template <typename T>
	struct user;

	template <typename T>
	struct light;


``sol::user<T>`` and ``sol::light<T>`` are two utility classes that do not participate in the full :doc:`sol::usertype\<T><usertype>` system. The goal of these classes is to provide the most minimal memory footprint and overhead for putting a single item and getting a single item out of Lua. ``sol::user<T>``, when pushed into Lua, will create a thin, unnamed metatable for that instance specifically which will be for calling its destructor. ``sol::light<T>`` specifically pushes a reference / pointer into Lua as a ``sol::type::lightuserdata``.

If you feel that you do not need to have something participate in the full :doc:`usertype\<T><usertype>` system, use the utility functions ``sol::make_user( ... )`` and ``sol::make_light( ... )`` to create these types and store them into Lua. You can get them off the Lua stack / out of the Lua system by using the same retrieval techniques on ``get`` and ``operator[]`` on tables and with stack operations.

Both have implicit conversion operators to ``T*`` and ``T&``, so you can set them immediately to their respective pointer and reference types if you need them.