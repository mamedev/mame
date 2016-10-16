simple_usertype
==================
structures and classes from C++ made available to Lua code (simpler)
--------------------------------------------------------------------


This type is no different from :doc:`regular usertype<usertype>`, but allows much of its work to be done at runtime instead of compile-time. You can reduce compilation times from a plain `usertype` when you have an exceedingly bulky registration listing.

You can set functions incrementally to reduce compile-time burden with ``simple_usertype`` as well, as shown in `this example`_.

Some developers used ``simple_usertype`` to have variables automatically be functions. To achieve this behavior, wrap the desired variable into :doc:`sol::as_function<as_function>`.

The performance `seems to be good enough`_ to not warn about any implications of having to serialize things at runtime. You do run the risk of using (slightly?) more memory, however, since variables and functions need to be stored differently and separately from the metatable data itself like with a regular ``usertype``. The goal here was to avoid compiler complaints about too-large usertypes (some individuals needed to register 190+ functions, and the compiler choked from the templated implementation of ``usertype``). As of Sol 2.14, this implementation has been heavily refactored to allow for all the same syntax and uses of usertype to apply here, with no caveats/exceptions.

.. _seems to be good enough: https://github.com/ThePhD/sol2/issues/202#issuecomment-246767629
.. _this example: https://github.com/ThePhD/sol2/blob/develop/examples/usertype_simple.cpp