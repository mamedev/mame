features
========
what does Sol (and other libraries) support?
--------------------------------------------

The goal of Sol is to provide an incredibly clean API that provides high performance (comparable or better than the C it was written on) and extreme ease of use. That is, users should be able to say: "this works pretty much how I expected it to."

For the hard technical components of Lua and its ecosystem we support, here is the full rundown:

what Sol supports
-----------------

* Support for Lua 5.1, 5.2, and 5.3. We achieve this through our "doc:`compatibility<compatibility>` header.

* :doc:`Table<api/table>` support: setting values, getting values of multiple (different) types
	- :doc:`Lazy evaluation<api/proxy>` for nested/chained queries
		``table["a"]["b"]["c"] = 24;``
	- Implicit conversion to the types you want
		``double b = table["computed_value"];``

* :doc:`Optional<api/optional>` support: setting values, getting values of multiple (different) types
	- :doc:`Lazy evaluation<api/proxy>` for nested/chained queries
		``optional<int> maybe_number = table["a"]["b"]["invalid_key"];``
	- Turns on safety when you want it: speed when you don't

* Support for callables (functions, lambdas, member functions)
 	- Pull out any Lua function with :doc:`sol::function<api/function>`
 		``sol::function fx = table["socket_send"];``
 	- Can also set callables into :doc:`operator[] proxies<api/proxy>`
 		``table["move_dude"] = engine::move_dude;``
 	- Safety: use :doc:`sol::protected_function<api/protected_function>` to catch any kind of error
 		+ ANY kind: C++ exception or Lua erors are trapped and run through the optional ``error_handler`` variable
 	- *Advanced:* Overloading of a single function so you don't need to do boring typechecks

* User-Defined Type (:doc:`sol::usertype<api/usertype>` in the API) support:
	- Set member functions to be called
	- Set member variables
	- Set variables on a class that are based on setter/getter functions
	- Use free-functions that take the Type as a first argument (pointer or reference)
	- Support for "Factory" classes that do not expose constructor or destructor
	- Modifying memory of userdata in C++ directly affects Lua without copying, and
	- Modifying userdata in Lua directly affects C++ references/pointers
		``my_class& a = table["a"];`` 
		``my_class* a_ptr = table["a"];`` 
	- If you want a copy, just use value semantics and get copies:
		``my_class a = table["a"];``

* Thread/Coroutine support
	- Use, resume, and play with :doc:`coroutines<api/coroutine>` like regular functions
	- Get and use them even on a separate Lua :doc:`thread<api/thread>` 
	- Monitor status and get check errors

* *Advanced:* Customizable and extensible to your own types if you override :doc:`getter/pusher/checker<api/stack>` template struct definitions.


The Feature Matrix™
-------------------

The below feature table checks for the presence of something. It, however, does not actually account for any kind of laborious syntax.

✔ full support: works as you'd expect (operator[] on tables, etc...)

~ partial support / wonky support: this means its either supported through some other fashion (not with the desired syntax, serious caveats, etc.). Sometimes means dropping down to use the plain C API (at which point, what was the point of the abstraction?).

✗ no support: feature doesn't work or, if it's there, it REALLY sucks to use

Implementation notes from using the libraries are below the tables.


category explanations
---------------------

Explanations for a few categories are below (rest are self-explanatory).

* optional: Support for getting an element, or potentially not (and not forcing the default construction of what amounts to a bogus/dead object). Usually comes with ``std(::experimental)::optional``. It's a fairly new class, so a hand-rolled class internal to the library with similar semantics is also acceptable
* tables: Some sort of abstraction for dealing with tables. Ideal support is ``mytable["some_key"] = value``, and everything that the syntax implies.
* table chaining: In conjunction with tables, having the ability to query deeply into tables ``mytable["key1"]["key2"]["key3"]``. Note that this becomes a tripping point for some libraries: crashing if ``"key1"`` doesn't exist while trying to access ``"key2"`` (Sol avoids this specifically when you use ``sol::optional``), and sometimes it's also a heavy performance bottleneck as expressions are not lazy-evaluated by a library.
* arbitrary keys: Letting C++ code use userdata, other tables, integers, etc. as keys for into a table.
* user-defined types (udts): C++ types given form and function in Lua code.
* udts - member functions: C++ member functions on a type, usually callable with ``my_object:foo(1)`` or similar in Lua.
* udts - table variables: C++ member variables/properties, manipulated by ``my_object.var = 24`` and in Lua
* function binding: Support for binding all types of functions. Lambdas, member functions, free functions, in different contexts, etc... 
* protected function: Use of ``lua_pcall`` to call a function, which offers error-handling and trampolining (as well as the ability to opt-in / opt-out of this behavior)
* multi-return: returning multiple values from and to Lua (generally through ``std::tuple<...>`` or in some other way)
* variadic/variant argument: being able to accept "anything" from Lua, and even return "anything" to Lua (``object`` abstraction, variadic arguments, etc...)
* inheritance: allowing some degree of subtyping or inheritance on classes / userdata from Lua - this generally means that you can retrieve a base pointer from Lua even if you hand the library a derived pointer
* overloading: the ability to call overloaded functions, matched based on arity or type (``foo( 1 )`` from lua calls a different function then ``foo( "bark" )``).
* Lua thread: basic wrapping of the lua thread API; ties in with coroutine.
* coroutines: allowing a function to be called multiple times, resuming the execution of a Lua coroutine each time

+---------------------------+-------------+------------+----------+---------+----------+-----------+-----------+----------------+----------+----------+-----------+-----------------+--------+
|                           |   plain C   | luawrapper | lua-intf | luabind |  Selene  |    Sol2   |   oolua   |   lua-api-pp   |  kaguya  |   SLB3   |    SWIG   | luacppinterface | luwra  |
|                           |             |            |          |         |          |           |           |                |          |          |           |                 |        |
+===========================+=============+============+==========+=========+==========+===========+===========+================+==========+==========+===========+=================+========+
| optional                  |      ~      |     ✗      |     ✔    |    ✗    |     ✗    |     ✔     |     ✗     |        ✗       |     ✔    |     ✗    |     ✗     |        ✗        |    ✗   |
+---------------------------+-------------+------------+----------+---------+----------+-----------+-----------+----------------+----------+----------+-----------+-----------------+--------+
| tables                    |      ~      |     ~      |     ~    |    ✔    |     ✔    |     ✔     |     ~     |        ✔       |     ✔    |     ✗    |     ✗     |        ~        |    ✔   |
+---------------------------+-------------+------------+----------+---------+----------+-----------+-----------+----------------+----------+----------+-----------+-----------------+--------+
| table chaining            |      ~      |     ~      |     ~    |    ✔    |     ✔    |     ✔     |     ✗     |        ✔       |     ✔    |     ✗    |     ✗     |        ~        |    ✔   |
+---------------------------+-------------+------------+----------+---------+----------+-----------+-----------+----------------+----------+----------+-----------+-----------------+--------+
| arbitrary keys            |      ~      |     ✔      |     ✔    |    ✔    |     ✔    |     ✔     |     ✗     |        ~       |     ✔    |     ✗    |     ✗     |        ✗        |    ✗   |
+---------------------------+-------------+------------+----------+---------+----------+-----------+-----------+----------------+----------+----------+-----------+-----------------+--------+
| user-defined types (udts) |      ~      |     ✔      |     ✔    |    ✔    |     ✔    |     ✔     |     ~     |        ✔       |     ✔    |     ✔    |     ✔     |        ✔        |    ✔   |
+---------------------------+-------------+------------+----------+---------+----------+-----------+-----------+----------------+----------+----------+-----------+-----------------+--------+
| udts: member functions    |      ~      |     ✔      |     ✔    |    ✔    |     ✔    |     ✔     |     ~     |        ✔       |     ✔    |     ✔    |     ✔     |        ✔        |    ✔   |
+---------------------------+-------------+------------+----------+---------+----------+-----------+-----------+----------------+----------+----------+-----------+-----------------+--------+
| udts: table variables     |      ~      |     ~      |     ~    |    ~    |     ~    |     ✔     |     ~     |        ~       |     ~    |     ✗    |     ✔     |        ✗        |    ~   |
+---------------------------+-------------+------------+----------+---------+----------+-----------+-----------+----------------+----------+----------+-----------+-----------------+--------+
| stack abstractions        |      ~      |     ✔      |     ✔    |    ✔    |     ✔    |     ✔     |     ✔     |        ✔       |     ✔    |     ~    |     ✗     |        ~        |    ✔   |
+---------------------------+-------------+------------+----------+---------+----------+-----------+-----------+----------------+----------+----------+-----------+-----------------+--------+
| lua callables from C(++)  |      ~      |     ✔      |     ✔    |    ✔    |     ✔    |     ✔     |     ✔     |        ✔       |     ✔    |     ✔    |     ✔     |        ✔        |    ~   |
+---------------------------+-------------+------------+----------+---------+----------+-----------+-----------+----------------+----------+----------+-----------+-----------------+--------+
| function binding          |      ~      |     ✔      |     ✔    |    ✔    |     ✔    |     ✔     |     ~     |        ~       |     ✔    |     ~    |     ~     |        ~        |    ✔   |
+---------------------------+-------------+------------+----------+---------+----------+-----------+-----------+----------------+----------+----------+-----------+-----------------+--------+
| protected call            |      ~      |     ✗      |     ~    |    ~    |     ~    |     ✔     |     ~     |        ✔       |     ~    |     ~    |     ~     |        ~        |    ~   |
+---------------------------+-------------+------------+----------+---------+----------+-----------+-----------+----------------+----------+----------+-----------+-----------------+--------+
| multi-return              |      ~      |     ✗      |     ✔    |    ✔    |     ✔    |     ✔     |     ~     |        ✔       |     ✔    |     ~    |     ✔     |        ~        |    ✗   |
+---------------------------+-------------+------------+----------+---------+----------+-----------+-----------+----------------+----------+----------+-----------+-----------------+--------+
| variadic/variant argument |      ~      |     ✔      |     ✔    |    ✔    |     ✔    |     ✔     |     ~     |        ✔       |     ✔    |     ~    |     ~     |        ~        |    ✗   |
+---------------------------+-------------+------------+----------+---------+----------+-----------+-----------+----------------+----------+----------+-----------+-----------------+--------+
| inheritance               |      ~      |     ✔      |     ✔    |    ✔    |     ✔    |     ✔     |     ~     |        ~       |     ✔    |     ~    |     ✔     |        ~        |    ✗   |
+---------------------------+-------------+------------+----------+---------+----------+-----------+-----------+----------------+----------+----------+-----------+-----------------+--------+
| overloading               |      ~      |     ✗      |     ✔    |    ✗    |     ✗    |     ✔     |     ✗     |        ✗       |     ✔    |     ✔    |     ✔     |        ✗        |    ✗   |
+---------------------------+-------------+------------+----------+---------+----------+-----------+-----------+----------------+----------+----------+-----------+-----------------+--------+
| Lua thread                |      ~      |     ✗      |     ~    |    ✗    |     ✗    |     ✔     |     ✔     |        ✗       |     ✔    |     ✗    |     ✗     |        ✔        |    ✗   |
+---------------------------+-------------+------------+----------+---------+----------+-----------+-----------+----------------+----------+----------+-----------+-----------------+--------+
| coroutines                |      ~      |     ✗      |     ~    |    ✔    |     ✔    |     ✔     |     ✗     |        ✗       |     ✔    |     ✗    |     ✗     |        ✔        |    ✗   |
+---------------------------+-------------+------------+----------+---------+----------+-----------+-----------+----------------+----------+----------+-----------+-----------------+--------+
| no-rtti support           |      ✔      |     ✗      |     ✔    |    ✗    |     ✗    |     ✔     |     ✔     |        ✗       |     ✔    |     ✔    |     ~     |        ✔        |    ✔   |
+---------------------------+-------------+------------+----------+---------+----------+-----------+-----------+----------------+----------+----------+-----------+-----------------+--------+
| no-exception support      |      ✔      |     ✗      |     ✔    |    ~    |     ✗    |     ✔     |     ✔     |        ✗       |     ✔    |     ✔    |     ~     |        ✔        |    ✔   |
+---------------------------+-------------+------------+----------+---------+----------+-----------+-----------+----------------+----------+----------+-----------+-----------------+--------+
| Lua 5.1                   |      ✔      |     ✔      |     ✔    |    ✔    |     ✗    |     ✔     |     ✔     |        ✔       |     ✔    |     ✔    |     ✔     |        ✗        |    ✔   |
+---------------------------+-------------+------------+----------+---------+----------+-----------+-----------+----------------+----------+----------+-----------+-----------------+--------+
| Lua 5.2                   |      ✔      |     ✔      |     ✔    |    ✔    |     ✔    |     ✔     |     ✔     |        ✔       |     ✔    |     ✔    |     ✔     |        ✔        |    ✔   |
+---------------------------+-------------+------------+----------+---------+----------+-----------+-----------+----------------+----------+----------+-----------+-----------------+--------+
| Lua 5.3                   |      ✔      |     ✔      |     ✔    |    ✔    |     ✔    |     ✔     |     ✔     |        ✔       |     ✔    |     ✔    |     ✔     |        ✔        |    ✔   |
+---------------------------+-------------+------------+----------+---------+----------+-----------+-----------+----------------+----------+----------+-----------+-----------------+--------+
| luajit                    |      ✔      |     ✔      |     ✔    |    ✔    |     ~    |     ✔     |     ✔     |        ✔       |     ✔    |     ✔    |     ✔     |        ✗        |    ✔   |
+---------------------------+-------------+------------+----------+---------+----------+-----------+-----------+----------------+----------+----------+-----------+-----------------+--------+
| distribution              |   compile   |   header   |   both   | compile |  header  |   header  |  compile  |     compile    |  header  |  compile | generated |     compile     | header |
+---------------------------+-------------+------------+----------+---------+----------+-----------+-----------+----------------+----------+----------+-----------+-----------------+--------+


notes on implementations
------------------------

Plain C - 

* Obviously you can do anything you want with Plain C, but the effort involved is astronomical in comparison to what frameworks offer
* Does not scale very well (in terms of developer ease of use)
* Compilation (or package manager use) is obviously required for your platform and required to use ANY of these libraries, but that's okay because all libraries need some version of Lua anyways, so you always have this!

kaguya -

* Table variables / member variables are automatically turned into ``obj:x( value )`` to set and ``obj:x()`` to get
* Has optional support
* Inspired coroutine support for Sol
* Library author (satoren) is a nice guy!
* C++11/14, or boostified (which makes it C++03 compatible)
* Class registration is a bit verbose, but not as offensive as OOLua or lua-intf or others

Sol -

* One of the few libraries with optional support!
* Basically the fastest in almomst all respects: http://sol2.readthedocs.io/en/latest/benchmarks.html
* Overloading support can get messy with inheritance, see :doc:`here<api/overload>`
* C++14/"C++1y" (-std=c++14, -std=c++1y, =std=c++1z) flags are used (available since GCC 4.9 and Clang 3.5)
* Active issues, active individuals
* Deserves lots of love!
  
lua-intf -

* Can be both header-only or compiled
* Has optional support
* C++11
* Macro-based registration (strange pseudo-language)
* Fairly fast in most regards
* Registering classes/"modules" in using C++ code is extremely verbose
* In order to chain lookups, one has to glue the keys together (e.g. ``"mykey.mykey2"``) on the ``operator[]`` lookup (e.g., you can't nest them arbitrarily, you have to pre-compose the proper lookup string) (fails miserably for non-string lookups!).
* Not too shabby!

Selene -

* Table variables / member variables are automatically turned into ``obj:set_x( value )`` to set and ``obj:x()`` to get
* Registering classes/"modules" using C++ code is extremely verbose, similar to lua-intf's style
* Eats crap when it comes to performance, most of the time (see :doc:`benchmarks<benchmarks>`)
* Lots of users (blogpost etc. made it popular), but the Repository is kinda stagnant...

luawrapper -

* Takes the approach of writing and reading tables using ``readVariable`` and ``writeVariable`` functions
* C++11, no macros!
* The interface can be clunky (no table-like data structures: most things go though ``readVariable`` / ``writeVariable``)
* Internal Compiler errors in Visual Studio 2015 - submitted a PR to fix it, hopefully it'll get picked up

SWIG (3.0) - 

* Very comprehensive for binding concepts of C++ (classes, variables, etc.) to Lua
* Helps with literally nothing else (tables, threads, stack abstractions, etc.)
* Not really a good, full-featured Library...
* Requires preprocessing step (but it's not a... TERRIBLY complicated preprocessing step); some boilerplate in writing additional classes that you've already declared

luacppinterface -

* The branch that fixes VC++ warnings and introduces some new work has type checker issues, so use the stable branch only
* No table variable support
* Actually has tables (but no operator[])
* Does not support arbitrary keys

luabind -

* One of the older frameworks, but has many people updating it and providing "deboostified" versions
* Strange in-lua keywords and parsing to allow for classes to be written in lua
	- not sure if good feature; vendor lock-in to that library to depend on this specific class syntax?
* Comprehensive lua bindings (can even bind "properties")
* There's some code that produces an ICE in Visual C++: I submitted a fix to the library in the hopes that it'll get accepted
* Wonky table support: no basic conversion functions on ``luabind::object``; have to push object then use lua API to get what you want

lua-api-pp -

* Compiled, but the recommendation is to add the source files directly to your project
* Userdata registration with thick setup-macros: LUAPP_USERDATA( ... ) plus a bunch of free functions that take a ``T& self`` argument
    - You can bind member functions directly but only if you override metatable entries
    - Otherwise, COMPLICATED self-registration that makes you wonder why you're using the framework
* You have to create a context and then call it to start accessing the lua state (adding more boilerplate... thanks)
    - Thankfully, unlike many libraries, it actually has a Table type that can be used semi-easily. FINALLY
* C++11-ish in some regards
* Sad face, thanks to the way userdata registration is handled

SLB3 -

* Old code exported to github from dying google code
* ".NET Style" - to override functionality, derive from class -- boilerplate (isn't that what we're trying to get rid of?)
* Pointers everywhere: ownership semantics unclear
* Piss-poor documentation, ugh!
* Least favorite to work with, for sure!

oolua -

* The syntax for this library. `Go read the docs`_ 
* The worst in terms of how to use it: may have docs, but the DSL is extraordinarily crappy with thick, hard-to-debug/hard-to-error-check macros
    - Same problem as lua-api-pp: cannot have the declaration macros anywhere but the toplevel namespace because of template declaration macro
* Supports not having exceptions or rtti turned on (shiny!)
* Poor RAII support: default-construct-and-get style (requires some form of initalization to perform a ``get`` of an object, and it's hard to extend)
	- The library author has informed me that he does personally advises individuals do not use the ``Table`` abstraction in OOLua... Do I likewise tell people to consider its table abstractions defunct?
* Table variables / member variables from C++ are turned into function calls (``get_x`` and ``set_x`` by default)

luwra - 

* How do you store stateful functors / lambas? So far, no support for such.
* Cannot pull functions without first leaving them on the stack: manual cleanup becomes a thing
* Doesn't understand ``std::function`` conversions and the like (but with some extra code can get it to work)
* Recently improved by a lot: can chain tables and such, even if performance is a bit sad for that use case
* When you do manage to set function calls with the macros they are fast (can a template solution do just as good? Sol is going to find out!)
* No table variable support - get turned into getter/setter functions, similar to kaguya
* Table variables become class statics (surprising)

.. _Go read the docs: https://oolua.org/docs/index.html
