exceptions
==========
since somebody is going to ask about it...
------------------------------------------

Yes, you can turn off exceptions in Sol with ``#define SOL_NO_EXCEPTIONS`` before including or by passing the command line argument that defines ``SOL_NO_EXCEPTIONS``. We don't recommend it unless you're playing with a Lua distro that also doesn't play nice with exceptions (like non-x64 versions of :ref:`LuaJIT<LuaJIT and exceptions>` ).

If you turn this off, the default `at_panic`_ function :doc:`state<api/state>` set for you will not throw. Instead, the default Lua behavior of aborting will take place (and give you no chance of escape unless you implement your own at_panic function and decide to try ``longjmp`` out).

To make this not be the case, you can set a panic function directly with ``lua_atpanic( lua, my_panic_function );`` or when you create the ``sol::state`` with ``sol::state lua(my_panic_function);``. Here's an example ``my_panic_function`` you can have that prints out its errors:

.. code-block:: cpp
	:caption: regular panic function

	#include <sol.hpp>
	#include <iostream>

	int my_panic_function( lua_State* L ) {
		// error message is at the top of the stack
		const char* message = lua_tostring(L, -1);
		// message can be null, so don't crash 
		// us with nullptr-constructed-string if it is
		std::string err = message ? message : "An unexpected error occurred and forced the lua state to call atpanic";
		// Weee
		std::cerr << err << std::endl;
		// When this function exits, Lua will exhibit default behavior and abort()
	}

	int main () {
		sol::state lua(my_panic_function);
	}


Note that ``SOL_NO_EXCEPTIONS`` will also disable :doc:`protected_function<api/protected_function>`'s ability to catch C++ errors you throw from C++ functions bound to Lua that you are calling through that API. So, only turn off exceptions in Sol if you're sure you're never going to use exceptions ever. Of course, if you are ALREADY not using Exceptions, you don't have to particularly worry about this and now you can use Sol!

If there is a place where a throw statement is called or a try/catch is used and it is not hidden behind a ``#ifndef SOL_NO_EXCEPTIONS`` block, please file an issue at `issue`_ or submit your very own pull request so everyone can benefit!


.. _LuaJIT and exceptions:

LuaJIT and exceptions
---------------------

It is important to note that a popular 5.1 distribution of Lua, LuaJIT, has some serious `caveats regarding exceptions`_. LuaJIT's exception promises are flaky at best on x64 (64-bit) platforms, and entirely terrible on non-x64 (32-bit, ARM, etc.) platorms. The trampolines we have in place for all functions bound through conventional means in Sol will catch exceptions and turn them into Lua errors so that LuaJIT remainds unperturbed, but if you link up a C function directly yourself and throw, chances are you might have screwed the pooch.

Testing in `this closed issue`_ that it doesn't play nice on 64-bit Linux in many cases either, especially when it hits an error internal to the interpreter (and does not go through Sol). We do have tests, however, that compile for our continuous integration check-ins that check this functionality across several compilers and platforms to keep you protected and given hard, strong guarantees for what happens if you throw in a function bound by Sol. If you stray outside the realm of Sol's protection, however... Good luck.

.. _issue: https://github.com/ThePhD/sol2/issues/
.. _at_panic: http://www.Lua.org/manual/5.3/manual.html#4.6
.. _caveats regarding exceptions: http://luajit.org/extensions.html#exceptions
.. _this closed issue: https://github.com/ThePhD/sol2/issues/28