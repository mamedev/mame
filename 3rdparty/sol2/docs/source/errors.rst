errors
======
how to handle exceptions or other errors 
----------------------------------------

Here is some advice and some tricks for common errors about iteration, compile time / linker errors, and other pitfalls, especially when dealing with thrown exceptions, error conditions and the like in Sol.


Linker Errors
-------------

There are lots of reasons for compiler linker errors. A common one is not knowing that you've compiled the Lua library as C++: when building with C++, it is important to note that every typical (static or dynamic) library expects the C calling convention to be used and that Sol includes the code using ``extern 'C'`` where applicable.

However, when the target Lua library is compiled with C++, one must change the calling convention and name mangling scheme by getting rid of the ``extern 'C'`` block. This can be achieved by adding ``#define SOL_USING_CXX_LUA`` before including sol2, or by adding it to your compilation's command line.


Catch and CRASH!
----------------

By default, Sol will add a ``default_at_panic`` handler. If exceptions are not turned off, this handler will throw to allow the user a chance to recover. However, in almost all cases, when Lua calls ``lua_atpanic`` and hits this function, it means that something *irreversibly wrong* occured in your code or the Lua code and the VM is in an unpredictable or dead state. Catching an error thrown from the default handler and then proceeding as if things are cleaned up or okay is NOT the best idea. Unexpected bugs in optimized and release mode builds can result, among other serious issues.

It is preferred if you catch an error that you log what happened, terminate the Lua VM as soon as possible, and then crash if your application cannot handle spinning up a new Lua state. Catching can be done, but you should understand the risks of what you're doing when you do it. For more information about catching exceptions, the potentials, not turning off exceptions and other tricks and caveats, read about :doc:`exceptions in Sol here<exceptions>`.

Lua is a C API first and foremost: exceptions bubbling out of it is essentially last-ditch, terminal behavior that the VM does not expect. You can see an example of handling a panic on the exceptions page :ref:`here<typical-panic-function>`.


Destructors and Safety
----------------------

Another issue is that Lua is a C API. It uses ``setjmp`` and ``longjmp`` to jump out of code when an error occurs. This means it will ignore destructors in your code if you use the library or the underlying Lua VM improperly. To solve this issue, build Lua as C++. When a Lua VM error occurs and ``lua_error`` is triggered, it raises it as an exception which will provoke proper unwinding semantics.


Protected Functions and Access
------------------------------

By default, :doc:`sol::function<api/function>` assumes the code ran just fine and there are no problems. :ref:`sol::state(_view)::script(_file)<state-script-function>` also assumes that code ran just fine. Use :doc:`sol::protected_function<api/protected_function>` to have function access where you can check if things worked out. Use :doc:`sol::optional<api/optional>` to get a value safely from Lua. Use :ref:`sol::state(_view)::do_string/do_file/load/load_file<state-do-code>` to safely load and get results from a script. The defaults are provided to be simple and fast with thrown exceptions to violently crash the VM in case things go wrong.

Protected Functions Are Not Catch All
-------------------------------------

Sometimes, some scripts load poorly. Even if you protect the function call, the actual file loading or file execution will be bad, in which case :doc:`sol::protected_function<api/protected_function>` will not save you. Make sure you register your own panic handler so you can catch errors, or follow the advice of the catch + crash behavior above.

Raw Functions
-------------

When you push a function into Lua using Sol using any methods and that function exactly matches the signature ``int( lua_State* );`` (and is a free function (e.g., not a member function pointer)), it will be treated as a *raw C function*. This means that the usual exception trampoline Sol wraps your other function calls in will not be present. You will be responsible for catching exceptions and handling them before they explode into the C API (and potentially destroy your code). Sol in all other cases adds an exception-handling trampoline that turns exceptions into Lua errors that can be caught by the above-mentioned protected functions and accessors.

.. warning::
	
	Do NOT assume that building Lua as C++ will allow you to throw directly from a raw function. If an exception is raised and it bubbles into the Lua framework, even if you compile as C++, Lua does not recognize exceptions other than the ones that it uses with ``lua_error``. In other words, it will return some completely bogus result, potentially leave your Lua stack thrashed, and the rest of your VM *can* be in a semi-trashed state. Please avoid this!


Iteration
---------

Tables may have other junk on them that makes iterating through their numeric part difficult when using a bland ``for-each`` loop, or when calling sol's ``for_each`` function. Use a numeric look to iterate through a table. Iteration does not iterate in any defined order also: see :ref:`this note in the table documentation for more explanation<iteration_note>`.
