stack_reference
===============
zero-overhead object on the stack
---------------------------------

When you work with a :doc:`sol::reference<reference>`, the object gotten from the stack has a reference to it made in the registry, keeping it alive. If you want to work with the Lua stack directly without having any additional references made, ``sol::stack_reference`` is for you. Its API is identical to ``sol::reference`` in every way, except it contains a ``int stack_index()`` member function that allows you to retrieve the stack index.

All of the base types have ``stack`` versions of themselves, and the APIs are identical to their non-stack forms. This includes :doc:`sol::stack_table<table>`, :doc:`sol::stack_function<function>`, :doc:`sol::stack_protected_function<protected_function>`, :doc:`sol::stack_(light\_)userdata<userdata>` and :doc:`sol::stack_object<object>`.