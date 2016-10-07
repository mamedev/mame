object
======
general-purpose safety reference to an existing object
------------------------------------------------------

.. code-block:: cpp
	
	class object : reference;


``object``'s goal is to allow someone to pass around the most generic form of a reference to something in Lua (or propogate a ``nil``). It is the logical extension of :doc:`sol::reference<reference>`, and is used in :ref:`sol::table's iterators<table-iterators>`.


members
-------

.. code-block:: cpp
    :caption: overloaded constructor: object
    :name: overloaded-object-constructor

    template <typename T>
    object(T&&);
    object(lua_State* L, int index = -1);
    template <typename T, typename... Args>
    object(lua_State* L, in_place_t, T&& arg, Args&&... args);
    template <typename T, typename... Args>
    object(lua_State* L, in_place_type_t<T>, Args&&... args);

There are 4 kinds of constructors here. One allows construction of an object from other reference types such as :doc:`sol::table<table>` and :doc:`sol::stack_reference<stack_reference>`. The second creates an object which references the specific element at the given index in the specified ``lua_State*``. The more advanced ``in_place...`` constructors create a single object by pushing the specified type ``T`` onto the stack and then setting it as the object. It gets popped from the stack afterwards (unless this is an instance of ``sol::stack_object``, in which case it is left on the stack). An example of using this and :doc:`sol::make_object<make_reference>` can be found in the `any_return example`_.

.. code-block:: cpp
	:caption: function: type conversion
	
	template<typename T>
	decltype(auto) as() const;

Performs a cast of the item this reference refers to into the type ``T`` and returns it. It obeys the same rules as :ref:`sol::stack::get\<T><getter>`.

.. code-block:: cpp
	:caption: function: type check
	
	template<typename T>
	bool is() const;

Performs a type check using the :ref:`sol::stack::check<checker>` api, after checking if the reference is valid.


non-members
-----------

.. code-block:: cpp
	:caption: functions: nil comparators

	bool operator==(const object& lhs, const nil_t&);
	bool operator==(const nil_t&, const object& rhs);
	bool operator!=(const object& lhs, const nil_t&);
	bool operator!=(const nil_t&, const object& rhs);

These allow a person to compare an ``sol::object`` against :ref:`nil<nil>`, which essentially checks if an object references a non-nil value, like so:

.. code-block:: cpp

	if (myobj == sol::nil) {
		// doesn't have anything...
	}

Use this to check objects.


.. _any_return example: https://github.com/ThePhD/sol2/blob/develop/examples/any_return.cpp