binary size, compile time
=========================
getting good final product out of sol2
--------------------------------------

For individiauls who use :doc:`usertypes<api/usertype>` a lot, they can find their compilation times increase. This is due to C++11 and C++14 not having very good facilities for handling template parameters and variadic template parameters. There are a few things in cutting-edge C++17 and C++Next that sol can use, but the problem is many people cannot work with the latest and greatest: therefore, we have to use older techniques that result in a fair amount of redundant function specializations that can be subject to the pickiness of the compiler's inlining and other such techniques.

what to do
----------

Here are some notes on achieving better compile-times without sacrificing too much performance:

* When you bind lots of usertypes, put them all in a *single* translation unit (one C++ file) so that it is not recompiled multiple times over, only to be discarded later by the linker.
	- Remember that the usertype binding ends up being serialized into the Lua state, so you never need them to appear in a header and cause that same compilation overhead for every compiled unit in your project.
* For extremely large usertypes, consider using :doc:`simple_usertype<api/simple_usertype>`.
	- It performs much more work at runtime rather than compile-time, and should still give comparative performance (but it loses out in some cases for variable bindings or when you bind all functions to a usertype).
* If you are developing a shared library, restrict your overall surface area by specifically and explicitly marking functions as visible and exported and leaving everything else as hidden or invisible by default


next steps
----------

The next step for Sol from a developer standpoint is to formally make the library a C++17 one. This would mean using Fold Expressions and several other things which will reduce compilation time drastically. Unfortunately, that means also boosting compiler requirements. While most wouldn't care, others are very slow to upgrade: finding the balance is difficult, and often we have to opt for backwards compatibility and fixes for bad / older compilers (of which there are many in the codebase already).

Hopefully, as things progress, we move things forward.
