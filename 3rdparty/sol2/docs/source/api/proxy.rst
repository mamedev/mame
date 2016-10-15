proxy, (protected\_)function_result - proxy_base derivatives
============================================================
``table[x]`` and ``function(...)`` conversion struct
----------------------------------------------------

.. code-block:: c++

	template <typename Recurring>
	struct proxy_base;

	template <typename Table, typename Key>
	struct proxy : proxy_base<...>;

	struct stack_proxy: proxy_base<...>;

	struct function_result : proxy_base<...>;

	struct protected_function_result: proxy_base<...>;


These classes provide implicit assignment operator ``operator=`` (for ``set``) and an implicit conversion operator ``operator T`` (for ``get``) to support items retrieved from the underlying Lua implementation, specifically :doc:`sol::table<table>` and the results of function calls on :doc:`sol::function<function>` and :doc:`sol::protected_function<protected_function>`.

.. _proxy:

proxy
-----

``proxy`` is returned by lookups into :doc:`sol::table<table>` and table-like entities. Because it is templated on key and table type, it would be hard to spell: you can capture it using the word ``auto`` if you feel like you need to carry it around for some reason before using it. ``proxy`` evaluates its arguments lazily, when you finally call ``get`` or ``set`` on it. Here are some examples given the following lua script. 

.. code-block:: lua
	:linenos:
	:caption: lua nested table script

	bark = { 
		woof = {
			[2] = "arf!" 
		} 
	}


After loading that file in or putting it in a string and reading the string directly in lua (see :doc:`state`), you can start kicking around with it in C++ like so:

.. code-block:: c++
	:linenos:

	sol::state lua;

	// produces proxy, implicitly converts to std::string, quietly destroys proxy
	std::string x = lua["bark"]["woof"][2];


``proxy`` lazy evaluation:

.. code-block:: c++
	:linenos:
	:caption: multi-get

	auto x = lua["bark"];
	auto y = x["woof"];
	auto z = x[2];
	// retrivies value inside of lua table above
	std::string value = z; // "arf!"
	// Can change the value later...
	z = 20;
	// Yay, lazy-evaluation!
	int changed_value = z; // now it's 20!


We don't recommend the above to be used across classes or between function: it's more of something you can do to save a reference to a value you like, call a script or run a lua function, and then get it afterwards. You can also set functions (and function objects :ref:`*<note 1>`) this way, and retrieve them as well.

.. code-block:: c++
	:linenos:

	lua["bark_value"] = 24;
	lua["chase_tail"] = floof::chase_tail; // chase_tail is a free function


members
-------

.. code-block:: c++
	:caption: functions: [overloaded] implicit conversion get
	:name: implicit-get

	requires( sol::is_primitive_type<T>::value == true )
	template <typename T>
	operator T() const;
	
	requires( sol::is_primitive_type<T>::value == false )
	template <typename T>
	operator T&() const;

Gets the value associated with the keys the proxy was generated and convers it to the type ``T``. Note that this function will always return ``T&``, a non-const reference, to types which are not based on :doc:`sol::reference<reference>` and not a :doc:`primitive lua type<types>`

.. code-block:: c++
	:caption: function: get a value
	:name: regular-get

	template <typename T>
	decltype(auto) get( ) const;

Gets the value associated with the keys and converts it to the type ``T``.

.. code-block:: c++
	:caption: function: optionally get a value
	:name: regular-get-or

	template <typename T, typename Otherwise>
	optional<T> get_or( Otherwise&& otherise ) const;

Gets the value associated with the keys and converts it to the type ``T``. If it is not of the proper type, it will return a ``sol::nullopt`` instead.

``operator[]`` proxy-only members
---------------------------------

.. code-block:: c++
	:caption: function: valid
	:name: proxy-valid

	bool valid () const;

Returns whether this proxy actually refers to a valid object. It uses :ref:`sol::stack::probe_get_field<stack-probe-get-field>` to determine whether or not its valid.

.. code-block:: c++
	:caption: functions: [overloaded] implicit set
	:name: implicit-set

	requires( sol::detail::Function<Fx> == false )
	template <typename T>
	proxy& operator=( T&& value );
	
	requires( sol::detail::Function<Fx> == true )
	template <typename Fx>
	proxy& operator=( Fx&& function );

Sets the value associated with the keys the proxy was generated with to ``value``. If this is a function, calls ``set_function``. If it is not, just calls ``set``. Does not exist on :ref:`function_result<function-result>` or :ref:`protected_function_result<protected-function-result>`. See :ref:`note<note 1>` for caveats.

.. code-block:: c++
	:caption: function: set a callable
	:name: regular-set-function

	template <typename Fx>
	proxy& set_function( Fx&& fx );

Sets the value associated with the keys the proxy was generated with to a function ``fx``. Does not exist on :ref:`function_result<function-result>` or :ref:`protected_function_result<protected-function-result>`.


.. code-block:: c++
	:caption: function: set a value
	:name: regular-set

	template <typename T>
	proxy& set( T&& value );

Sets the value associated with the keys the proxy was generated with to ``value``. Does not exist on :ref:`function_result<function-result>` or :ref:`protected_function_result<protected-function-result>`.

stack_proxy
-----------

``sol::stack_proxy`` is what gets returned by :doc:`sol::variadic_args<variadic_args>` and other parts of the framework. It is similar to proxy, but is meant to alias a stack index and not a named variable.

.. _function-result:

function_result
---------------

``function_result`` is a temporary-only, intermediate-only implicit conversion worker for when :doc:`function<function>` is called. It is *NOT* meant to be stored or captured with ``auto``. It provides fast access to the desired underlying value. It does not implement ``set`` / ``set_function`` / templated ``operator=``, as is present on :ref:`proxy<proxy>`.


.. _protected-function-result:

protected_function_result
-------------------------

``protected_function_result`` is a nicer version of ``function_result`` that can be used to detect errors. Its gives safe access to the desired underlying value. It does not implement ``set`` / ``set_function`` / templated ``operator=`` as is present on :ref:`proxy<proxy>`.


.. _note 1:

on function objects and proxies
-------------------------------

Consider the following:

.. code-block:: cpp
	:linenos:
	:caption: Note 1 Case

	struct doge {
		int bark;

		void operator()() {
			bark += 1;
		}
	};

	sol::state lua;
	lua["object"] = doge{}; // bind constructed doge to "object"
	// but it binds as a function

When you use the ``lua["object"] = doge{};`` from above, keep in mind that Sol detects if this is a function *callable with any kind of arguments*. Since ``doge`` has overriden ``return_type operator()( argument_types... )`` on itself, it results in satisfying the ``requires`` constraint from above. This means that if you have a user-defined type you want to bind as a :doc:`userdata with usertype semantics<usertype>` with this syntax, it might get bound as a function and not as a user-defined type (d'oh!). use ``lua["object"].set(doge)`` directly to avoid this, or ``lua["object"].set_function(doge{})`` to perform this explicitly.