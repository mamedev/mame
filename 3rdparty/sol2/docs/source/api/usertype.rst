usertype<T>
===========
structures and classes from C++ made available to Lua code
----------------------------------------------------------

*Note: ``T`` refers to the type being turned into a usertype.*

While other frameworks extend lua's syntax or create Data Structure Languages (DSLs) to create classes in lua, :doc:`Sol<../index>` instead offers the ability to generate easy bindings. These use metatables and userdata in lua for their implementation. If you need a usertype that is also extensible at runtime and has less compiler crunch to it, try the :doc:`simple version of this after reading these docs<simple_usertype>` Given this C++ class:

.. code-block:: cpp
	:linenos:
	
	struct ship {
		int bullets = 20;
		int life = 100;

		bool shoot () {
			if (bullets > 0) {
				--bullets;
				// successfully shot
				return true;
			}
			// cannot shoot
			return false;
		}

		bool hurt (int by) {
			life -= by;
			// have we died?
			return life < 1;
		}
	};

You can bind the it to Lua using the following C++ code:

.. code-block:: cpp
	:linenos:

	sol::state lua;

	lua.new_usertype<ship>( "ship", // the name of the class, as you want it to be used in lua
		// List the member functions you wish to bind:
		// "name_of_item", &class_name::function_or_variable
		"shoot", &ship::shoot,
		"hurt", &ship::hurt,
		// bind variable types, too
		"life", &ship::bullets
		// names in lua don't have to be the same as C++,
		// but it probably helps if they're kept the same,
		// here we change it just to show its possible
		"bullet_count", &ship::bullets
	);


Equivalently, you can also write:

.. code-block:: cpp
	:linenos:
	:emphasize-lines: 4,12

	sol::state lua;

	// Use constructor directly
	usertype<ship> shiptype(
		"shoot", &ship::shoot,
		"hurt", &ship::hurt,
		"life", &ship::bullets
		"bullet_count", &ship::bullets
	);

	// set usertype explicitly, with the given name
	lua.set_usertype<ship>( "ship", shiptype );

	// shiptype is now a useless skeleton type, just let it destruct naturally and don't use it again.


Note that here, because the C++ class is default-constructible, it will automatically generate a creation function that can be called in lua called "new" that takes no arguments. You can use it like this in lua code:

.. code-block:: lua
	:linenos:

	fwoosh = ship.new()
	-- note the ":" that is there: this is mandatory for member function calls
	-- ":" means "pass self" in Lua
	local success = fwoosh:shoot()
	local is_dead = fwoosh:hurt(20)
	-- check if it works
	print(is_dead) -- the ship is not dead at this point
	print(fwoosh.life .. "life left") -- 80 life left
	print(fwoosh.bullet_count) -- 19


There are more advanced use cases for how to create and use a usertype, which are all based on how to use its constructor (see below).

enumerations
------------

.. _meta_function_enum:

.. code-block:: cpp
	:caption: meta_function enumeration for names
	:linenos:

	enum class meta_function {
		construct,
		index,
		new_index,
		mode,
		call,
		metatable,
		to_string,
		length,
		unary_minus,
		addition,
		subtraction,
		multiplication,
		division,
		modulus,
		power_of,
		involution = power_of,
		concatenation,
		equal_to,
		less_than,
		less_than_or_equal_to,
		garbage_collect,
		call_function,
	};


Use this enumeration to specify names in a manner friendlier than memorizing the special lua metamethod names for each of these. Each binds to a specific operation indicated by the descriptive name of the enum.

members
-------

.. code-block:: cpp
	:caption: function: usertype<T> constructor
	:name: usertype-constructor

	template<typename... Args>
	usertype<T>(Args&&... args);


The constructor of usertype takes a variable number of arguments. It takes an even number of arguments (except in the case where the very first argument is passed as the :ref:`constructor list type<constructor>`). Names can either be strings, :ref:`special meta_function enumerations<meta_function_enum>`, or one of the special indicators for initializers.


usertype constructor options
++++++++++++++++++++++++++++

If you don't specify any constructor options at all and the type is `default_constructible`_, Sol will generate a ``new`` for you. Otherwise, the following are special ways to handle the construction of a usertype:
 
..  _constructor:

* ``"{name}", constructors<Type-List-0, Type-List-1, ...>``
	- ``Type-List-N`` must be a ``sol::types<Args...>``, where ``Args...`` is a list of types that a constructor takes. Supports overloading by default
	- If you pass the ``constructors<...>`` argument first when constructing the usertype, then it will automatically be given a ``"{name}"`` of ``"new"``
* ``"{name}", sol::initializers( func1, func2, ... )``
	- Used to handle *initializer functions* that need to initialize the memory itself (but not actually allocate the memory, since that comes as a userdata block from Lua)
	- Given one or more functions, provides an overloaded Lua function for creating a the specified type
		+ The function must have the argument signature ``func( T*, Arguments... )`` or ``func( T&, Arguments... )``, where the pointer or reference will point to a place of allocated memory that has an uninitialized ``T``. Note that Lua controls the memory, so performing a ``new`` and setting it to the ``T*`` or ``T&`` is a bad idea: instead, use ``placement new`` to invoke a constructor, or deal with the memory exactly as you see fit
* ``{anything}, sol::factories( func1, func2, ... )``
	- Used to indicate that a factory function (e.g., something that produces a ``std::unique_ptr<T, ...>``, ``std::shared_ptr<T>``, ``T``, or similar) will be creating the object type
	- Given one or more functions, provides an overloaded function for invoking
		+ The functions can take any form and return anything, since they're just considered to be some plain function and no placement new or otherwise needs to be done. Results from this function will be pushed into Lua according to the same rules as everything else.
		+ Can be used to stop the generation of a ``.new()`` default constructor since a ``sol::factories`` entry will be recognized as a constructor for the usertype
		+ If this is not sufficient, see next 2 entries on how to specifically block a constructor
* ``{anything}, sol::no_constructor``
	- Specifically tells Sol not to create a ``.new()`` if one is not specified and the type is default-constructible
	- ``{anything}`` should probably be ``"new"``, which will specifically block its creation and give a proper warning if someone calls ``new`` (otherwise it will just give a nil value error)
	- *Combine with the next one to only allow a factory function for your function type*
* ``{anything}, {some_factory_function}``
	- Essentially binds whatever the function is to name ``{anything}``
	- When used WITH the ``sol::no_constructor`` option above (e.g. ``"new", sol::no_constructor`` and after that having ``"create", &my_creation_func``), one can remove typical constructor avenues and then only provide specific factory functions. Note that this combination is similar to using the ``sol::factories`` method mentioned earlier in this list. To control the destructor as well, see further below
* ``sol::call_constructor, {valid function / constructor / initializer / factory}``
	- The purpose of this is to enable the syntax ``local v = my_class( 24 )`` and have that call a constructor; it has no other purpose
	- This is compatible with luabind, kaguya and other Lua library syntaxes and looks similar to C++ syntax, but the general consensus in Programming with Lua and other places is to use a function named ``new``

usertype destructor options
+++++++++++++++++++++++++++

If you don't specify anything at all and the type is `destructible`_, then a destructor will be bound to the garbage collection metamethod. Otherwise, the following are special ways to handle the destruction of a usertype:

* ``"__gc", sol::destructor( func )`` or ``sol::meta_function::garbage_collect, sol::destructor( func )``
	- Creates a custom destructor that takes an argument ``T*`` or ``T&`` and expects it to be destructed/destroyed. Note that lua controls the memory and thusly will deallocate the necessary space AFTER this function returns (e.g., do not call ``delete`` as that will attempt to deallocate memory you did not ``new``)
	- If you just want the default constructor, you can replace the second argument with ``sol::default_destructor``
	- The usertype will error / throw if you specify a destructor specifically but do not map it to ``sol::meta_function::gc`` or a string equivalent to ``"__gc"``

usertype regular function options
+++++++++++++++++++++++++++++++++

If you don't specify anything at all and the type ``T`` supports ``operator <``, ``operator <=``, or ``operator==`` (``const`` or non-``const`` qualified):

* for ``operator <`` and ``operator <=`` 
	- These two ``sol::meta_function::less_than(_or_equal_to)`` are generated for you and overriden in Lua.
* for ``operator==``
	- An equality operator will always be generated, doing pointer comparison if ``operator==`` on the two value types is not supported or doing a reference comparison and a value comparison if ``operator==`` is supported
* heterogenous operators cannot be supported for equality, as Lua specifically checks if they use the same function to do the comparison: if they do not, then the equality method is not invoked; one way around this would be to write one ``int super_equality_function(lua_State* L) { ... }``, pull out arguments 1 and 2 from the stack for your type, and check all the types and then invoke ``operator==`` yourself after getting the types out of Lua (possibly using :ref:`sol::stack::get<stack-get>` and :ref:`sol::stack::check_get<stack-check-get>`)

Otherwise, the following is used to specify functions to bind on the specific usertype for ``T``.

* ``"{name}", &free_function``
	- Binds a free function / static class function / function object (lambda) to ``"{name}"``. If the first argument is ``T*`` or ``T&``, then it will bind it as a member function. If it is not, it will be bound as a "static" function on the lua table
* ``"{name}", &type::function_name`` or ``"{name}", &type::member_variable``
	- Binds a typical member function or variable to ``"{name}"``. In the case of a member variable or member function, ``type`` must be ``T`` or a base of ``T``
* ``"{name}", sol::readonly( &type::member_variable )``
	- Binds a typical variable to ``"{name}"``. Similar to the above, but the variable will be read-only, meaning an error will be generated if anything attemps to write to this variable
* ``"{name}", sol::as_function( &type::member_variable )``
	- Binds a typical variable to ``"{name}"`` *but forces the syntax to be callable like a function*. This produces a getter and a setter accessible by ``obj:name()`` to get and ``obj::name(value)`` to set.
* ``"{name}", sol::property( getter_func, setter_func )``
	- Binds a typical variable to ``"{name}"``, but gets and sets using the specified setter and getter functions. Not that if you do not pass a setter function, the variable will be read-only. Also not that if you do not pass a getter function, it will be write-only
* ``"{name}", sol::var( some_value )`` or ``"{name}", sol::var( std::ref( some_value ) )``
	- Binds a typical variable to ``"{name}"``, optionally by reference (e.g., refers to the same memory in C++). This is useful for global variables / static class variables and the like
* ``"{name}", sol::overloaded( Func1, Func2, ... )``
	- Creates an oveloaded member function that discriminates on number of arguments and types.
* ``sol::base_classes, sol::bases<Bases...>``
	- Tells a usertype what its base classes are. You need this to have derived-to-base conversions work properly. See :ref:`inheritance<usertype-inheritance>`


usertype arguments - simple usertype
++++++++++++++++++++++++++++++++++++

* ``sol::simple``
	- Only allowed as the first argument to the usertype constructor, must be accompanied by a ``lua_State*``
	- This tag triggers the :doc:`simple usertype<simple_usertype>` changes / optimizations
	- Only supported when directly invoking the constructor (e.g. not when calling ``sol::table::new_usertype`` or ``sol::table::new_simple_usertype``)
	- Should probably not be used directly. Use ``sol::table::new_usertype`` or ``sol::table::new_simple_usertype`` instead



overloading
-----------

Functions set on a usertype support overloading. See :doc:`here<overload>` for an example.


.. _usertype-inheritance:

inheritance
-----------

Sol can adjust pointers from derived classes to base classes at runtime, but it has some caveats based on what you compile with:

If your class has no complicated™ virtual inheritance or multiple inheritance, than you can try to sneak away with a performance boost from not specifying any base classes and doing any casting checks. (What does "complicated™" mean? Ask your compiler's documentation, if you're in that deep.)

For the rest of us safe individuals out there: You must specify the ``sol::base_classes`` tag with the ``sol::bases<Types...>()`` argument, where ``Types...`` are all the base classes of the single type ``T`` that you are making a usertype out of.

.. note::

	Always specify your bases if you plan to retrieve a base class using the Sol abstraction directly and not casting yourself.

.. code-block:: cpp
	:linenos:

	struct A { 
		int a = 10;
		virtual int call() { return 0; } 
	};
	struct B : A { 
		int b = 11; 
		virtual int call() override { return 20; } 
	};

Then, to register the base classes explicitly:

.. code-block:: cpp
	:linenos:
	:emphasize-lines: 5

	sol::state lua;

	lua.new_usertype<B>( "B",
		"call", &B::call,
		sol::base_classes, sol::bases<A>()
	);

.. note::

	You must list ALL base classes, including (if there were any) the base classes of A, and the base classes of those base classes, etc. if you want Sol/Lua to handle them automagically.

.. note::
	
	Sol does not support down-casting from a base class to a derived class at runtime.

.. warning::

	Specify all base class member variables and member functions to avoid current implementation caveats regarding automatic base member lookup. Sol currently attempts to link base class methods and variables with their derived classes with an undocumented, unsupported feature, provided you specify ``sol::base_classes<...>``. Unfortunately, this can come at the cost of performance, depending on how "far" the base is from the derived class in the bases lookup list. If you do not want to suffer the performance degradation while we iron out the kinks in the implementation (and want it to stay performant forever), please specify all the base methods on the derived class in the method listing you write. In the future, we hope that with reflection we will not have to worry about this.


inheritance + overloading
-------------------------

While overloading is supported regardless of inheritance caveats or not, the current version of Sol has a first-match, first-call style of overloading when it comes to inheritance. Put the functions with the most derived arguments first to get the kind of matching you expect or cast inside of an intermediary C++ function and call the function you desire.

traits
------

.. code-block:: cpp
	:caption: usertype_traits<T>
	:name: usertype-traits

	template<typename T>
	struct usertype_traits {
		static const std::string name;
		static const std::string metatable;
		static const std::string variable_metatable;
		static const std::string gc_table;
	};


This trait is used to provide names for the various metatables and global tables used to perform cleanup and lookup. They are automagically generated at runtime. Sol attempts to parse the output of ``__PRETTY_FUCNTION__`` (``g++``/``clang++``) or ``_FUNCDSIG`` (``vc++``) to get the proper type name. If you have a special need you can override the names for your specific type. If you notice a bug in a class name when you don't manually specify it during setting a usertype, feel free to open an issue request or send an e-mail!


compilation speed
-----------------

.. note::

	If you find that compilation times are too long and you're only binding member functions, consider perhaps using :doc:`simple usertypes<simple_usertype>`. This can reduce compile times (but may cost memory size and speed). See the simple usertypes documentation for more details.


performance note
----------------

.. note::

	Note that performance for member function calls goes down by a fixed overhead if you also bind variables as well as member functions. This is purely a limitation of the Lua implementation and there is, unfortunately, nothing that can be done about it. If you bind only functions and no variables, however, Sol will automatically optimize the Lua runtime and give you the maximum performance possible. *Please consider ease of use and maintenance of code before you make everything into functions.*


.. _destructible: http://en.cppreference.com/w/cpp/types/is_destructible
.. _default_constructible: http://en.cppreference.com/w/cpp/types/is_constructible