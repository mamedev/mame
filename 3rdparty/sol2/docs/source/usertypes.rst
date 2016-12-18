usertypes
=========

Perhaps the most powerful feature of sol2, ``usertypes`` are the way sol2 and C++ communicate your classes to the Lua runtime and bind things between both tables and to specific blocks of C++ memory, allowing you to treat Lua userdata and other things like classes. 

To learn more about usertypes, visit:

* :doc:`the basic tutorial<tutorial/cxx-in-lua>`
* :doc:`customization point tutorial<tutorial/customization>`
* :doc:`api documentation<api/usertype>`
* :doc:`memory documentation<api/usertype_memory>`

The examples folder also has a number of really great examples for you to see. There are also some notes about guarantees you can find about usertypes, and their associated userdata, below:

* You can push types classified as userdata before you register a usertype.
	- You can register a usertype with the Lua runtime at any time sol2
	- You can retrieve them from the Lua runtime as well through sol2
	- Methods and properties will be added to the type only after you register it in the Lua runtime
* Types either copy once or move once into the memory location, if it is a value type. If it is a pointer, we store only the reference.
	- This means take arguments of class types (not primitive types like strings or integers) by ``T&`` or ``T*`` to modify the data in Lua directly, or by plain ``T`` to get a copy
	- Return types and passing arguments to ``sol::function`` use perfect forwarding and reference semantics, which means no copies happen unless you specify a value explicitly. See :ref:`this note for details<function-argument-handling>`.
* The first ``sizeof( void* )`` bytes is always a pointer to the typed C++ memory. What comes after is based on what you've pushed into the system according to :doc:`the memory specification for usertypes<api/usertype_memory>`. This is compatible with a number of systems.
* Member methods, properties, variables and functions taking ``self&`` arguments modify data directly
	- Work on a copy by taking or returning a copy by value.
* The actual metatable associated with the usertype has a long name and is defined to be opaque by the Sol implementation.
* Containers get pushed as special usertypes, but can be disabled if problems arising as detailed :doc:`here<api/containers>`.
* You can use bitfields but it requires some finesse on your part. We have an example to help you get started `here that uses a few tricks`_.

.. _here that uses a few tricks: https://github.com/ThePhD/sol2/blob/develop/examples/usertype_bitfields.cpp
