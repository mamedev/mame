make_object/make_reference
==========================
Create a value on the Lua stack and return it 
---------------------------------------------

.. code-block:: cpp
	:caption: function: make_reference
	:name: make-reference

	template <typename R = reference, bool should_pop = (R is not base of sol::stack_index), typename T>
	R make_reference(lua_State* L, T&& value);
	template <typename T, typename R = reference, bool should_pop = (R is base of sol::stack_index), typename... Args>
	R make_reference(lua_State* L, Args&&... args);

Makes an ``R`` out of the value. The first overload deduces the type from the passed in argument, the second allows you to specify a template parameter and forward any relevant arguments to ``sol::stack::push``. The type figured out for ``R`` is what is referenced from the stack. This allows you to request arbitrary pop-able types from Sol and have it constructed from ``R(lua_State* L, int stack_index)``. If the template boolean ``should_pop`` is ``true``, the value that was pushed will be popped off the stack. It defaults to popping, but if it encounters a type such as :doc:`sol::stack_reference<stack_reference>` (or any of its typically derived types in Sol), it will leave the pushed values on the stack.

.. code-block:: cpp
	:caption: function: make_object
	:name: make-object

	template <typename T>
	object make_object(lua_State* L, T&& value);
	template <typename T, typename... Args>
	object make_object(lua_State* L, Args&&... args);

Makes an object out of the value. It pushes it onto the stack, then pops it into the returned ``sol::object``. The first overload deduces the type from the passed in argument, the second allows you to specify a template parameter and forward any relevant arguments to ``sol::stack::push``. The implementation essentially defers to :ref:`sol::make_reference<make-reference>` with the specified arguments, ``R == object`` and ``should_pop == true``. It is preferred that one uses the :ref:`in_place object constructor instead<overloaded-object-constructor>`, since it's probably easier to deal with, but both versions will be supported for forever, since there's really no reason not to and people already have dependencies on ``sol::make_object``.
