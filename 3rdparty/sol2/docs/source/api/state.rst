state
=====
owning and non-owning state holders for registry and globals
------------------------------------------------------------

.. code-block:: cpp

	class state_view;
	
	class state : state_view, std::unique_ptr<lua_State*, deleter>;

The most important class here is ``state_view``. This structure takes a ``lua_State*`` that was already created and gives you simple, easy access to Lua's interfaces without taking ownership. ``state`` derives from ``state_view``, inheriting all of this functionality, but has the additional purpose of creating a fresh ``lua_State*`` and managing its lifetime for you in the default constructor.

The majority of the members between ``state_view`` and :doc:`sol::table<table>` are identical, with added for this higher-level type. Therefore, all of the examples and notes in :doc:`sol::table<table>` apply here as well.

enumerations
------------

.. code-block:: cpp
	:caption: in-lua libraries
	:name: lib-enum

	enum class lib : char {
	    base,
	    package,
	    coroutine,
	    string,
	    os,
	    math,
	    table,
	    debug,
	    bit32,
	    io,
	    ffi,
	    jit,
	    count // do not use
	};

This enumeration details the various base libraries that come with Lua. See the `standard lua libraries`_ for details about the various standard libraries.


members
-------

.. code-block:: cpp
	:caption: function: open standard libraries/modules
	:name: open-libraries

	template<typename... Args>
	void open_libraries(Args&&... args);

This function takes a number of :ref:`sol::lib<lib-enum>` as arguments and opens up the associated Lua core libraries.

.. code-block:: cpp
	:caption: function: script / script_file

	sol::function_result script(const std::string& code);
	sol::function_result script_file(const std::string& filename);

These functions run the desired blob of either code that is in a string, or code that comes from a filename, on the ``lua_State*``. It will not run isolated: any scripts or code run will affect code in the ``lua_State*`` the object uses as well (unless ``local`` is applied to a variable declaration, as specified by the Lua language). Code ran in this fashion is not isolated. If you need isolation, consider creating a new state or traditional Lua sandboxing techniques.

If your script returns a value, you can capture it from the returned :ref:`function_result<function-result>`.

.. code-block:: cpp
	:caption: function: require / require_file
	:name: state-require-function

	sol::object require(const std::string& key, lua_CFunction open_function, bool create_global = true);
	sol::object require_script(const std::string& key, const std::string& code, bool create_global = true);
	sol::object require_file(const std::string& key, const std::string& file, bool create_global = true);

These functions play a role similar to `luaL_requiref`_ except that they make this functionality available for loading a one-time script or a single file. The code here checks if a module has already been loaded, and if it has not, will either load / execute the file or execute the string of code passed in. If ``create_global`` is set to true, it will also link the name ``key`` to the result returned from the open function, the code or the file. Regardless or whether a fresh load happens or not, the returned module is given as a single :doc:`sol::object<object>` for you to use as you see fit.

Thanks to `Eric (EToreo) for the suggestion on this one`_!

.. code-block:: cpp
	:caption: function: load / load_file
	:name: state-load-code

	sol::load_result load(const std::string& code);
	sol::load_result load_file(const std::string& filename);

These functions *load* the desired blob of either code that is in a string, or code that comes from a filename, on the ``lua_State*``. It will not run: it returns a ``load_result`` proxy that can be called to actually run the code, turned into a ``sol::function``, a ``sol::protected_function``, or some other abstraction. If it is called, it will run on the object's current ``lua_State*``: it is not isolated. If you need isolation, consider creating a new state or traditional Lua sandboxing techniques.

.. code-block:: cpp
	:caption: function: global table / registry table

	sol::global_table globals() const;
	sol::table registry() const;

Get either the global table or the Lua registry as a :doc:`sol::table<table>`, which allows you to modify either of them directly. Note that getting the global table from a ``state``/``state_view`` is usually unnecessary as it has all the exact same functions as a :doc:`sol::table<table>` anyhow.


.. code-block:: cpp
	:caption: function: Lua set_panic
	:name: set-panic

	void set_panic(lua_CFunction panic);

Overrides the panic function Lua calls when something unrecoverable or unexpected happens in the Lua VM. Must be a function of the that matches the ``int(*)(lua_State*)`` function signature.

.. code-block:: cpp
	:caption: function: make a table

	sol::table create_table(int narr = 0, int nrec = 0);
	template <typename Key, typename Value, typename... Args>
	sol::table create_table(int narr, int nrec, Key&& key, Value&& value, Args&&... args);


	template <typename... Args>
	sol::table create_table_with(Args&&... args);
	
	static sol::table create_table(lua_State* L, int narr = 0, int nrec = 0);
	template <typename Key, typename Value, typename... Args>
	static sol::table create_table(lua_State* L, int narr, int nrec, Key&& key, Value&& value, Args&&... args);

Creates a table. Forwards its arguments to :ref:`table::create<table-create>`.

.. _standard lua libraries: http://www.lua.org/manual/5.3/manual.html#6 
.. _luaL_requiref: https://www.lua.org/manual/5.3/manual.html#luaL_requiref
.. _Eric (EToreo) for the suggestion on this one: https://github.com/ThePhD/sol2/issues/90