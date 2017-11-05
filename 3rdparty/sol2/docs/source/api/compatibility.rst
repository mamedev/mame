compatibility.hpp
=================
Lua 5.3/5.2 compatibility for Lua 5.1/LuaJIT
--------------------------------------------

This is a detail header used to maintain compatability with the 5.2 and 5.3 APIs. It contains code from the MIT-Licensed `Lua code`_ in some places and also from the `lua-compat`_ repository by KeplerProject.

It is not fully documented as this header's only purpose is for internal use to make sure Sol compiles across all platforms / distributions with no errors or missing Lua functionality. If you think there's some compatibility features we are missing or if you are running into redefinition errors, please make an `issue in the issue tracker`_.

If you have this already in your project or you have your own compatibility layer, then please ``#define SOL_NO_COMPAT 1`` before including ``sol.hpp`` or pass this flag on the command line to turn off the compatibility wrapper.

For the licenses, see :doc:`here<../licenses>`

.. _issue in the issue tracker: https://github.com/ThePhD/sol2/issues/
.. _Lua code: http://www.Lua.org/
.. _lua-compat: https://github.com/keplerproject/lua-compat-5.3