integrating into existing code
==============================

If you're already using lua and you just want to use ``sol`` in some places, you can use ``state_view``:

.. code-block:: cpp
	:linenos:
	:caption: using state_view
	:name: state-view-snippet

	void something_in_my_system (lua_State* L) {
		// start using Sol with a pre-existing system
		sol::state_view lua(L); // non-owning

		lua.script("print('bark bark bark!')");

		sol::table expected_table(L); // get the table off the top of the stack
		// start using it... 
	}

:doc:`sol::state_view<../api/state>` is exactly like ``sol::state``, but it doesn't manage the lifetime of a ``lua_State*``. Therefore, you get all the goodies that come with a ``sol::state`` without any of  the ownership implications. Sol has no initialization components that need to deliberately remain alive for the duration of the program. It's entirely self-containing and uses lua's garbage collectors and various implementation techniques to require no state C++-side. After you do that, all of the power of `Sol` is available to you, and then some!

You may also want to call ``require`` and supply a string of a script file or something that returns an object that you set equal to something in C++. For that, you can use the :ref:`require functionality<state-require-function>`.

Remember that Sol can be as lightweight as you want it: almost all of Sol's types take the ``lua_State*`` argument and then a second ``int index`` stack index argument, meaning you can use :doc:`tables<../api/table>`, :doc:`lua functions<../api/function>`, :doc:`coroutines<../api/coroutine>`, and other reference-derived objects that expose the proper constructor for your use. You can also set :doc:`usertypes<../api/usertype>` and other things you need without changing your entire architecture.

Note that you can also make non-standard pointer and reference types with custom reference counting and such also play nice with the system. See :doc:`unique_usertype_traits\<T><../api/unique_usertype_traits>` to see how! Custom types is also mentioned in the :doc:`customization tutorial<customization>`.