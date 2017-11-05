ownership
=========

Sol will not take ownership of raw pointers: raw pointers do not own anything.

.. code-block:: cpp

	struct my_type {
		void stuff () {}
	};

	sol::state lua;

	// AAAHHH BAD
	// dangling pointer!
	lua["my_func"] = []() -> my_type* {
		return new my_type();
	};

	// AAAHHH!
	lua.set("something", new my_type());

	// AAAAAAHHH!!!
	lua["something_else"] = new my_type();

Use/return a ``unique_ptr`` or ``shared_ptr`` instead or just return a value:

.. code-block:: cpp

	// :ok:
	lua["my_func"] = []() -> std::unique_ptr<my_type> {
		return std::make_unique<my_type>();
	};

	// :ok:
	lua["my_func"] = []() -> std::shared_ptr<my_type> {
		return std::make_shared<my_type>();
	};

	// :ok:
	lua["my_func"] = []() -> my_type {
		return my_type();
	};

	// :ok: 
	lua.set("something", std::unique_ptr<my_type>(new my_type()));

	std::shared_ptr<my_type> my_shared = std::make_shared<my_type>();
	// :ok: 
	lua.set("something_else", my_shared);

	auto my_unique = std::make_unique<my_type>();
	lua["other_thing"] = std::move(my_unique);

If you have something you know is going to last and you just want to give it to Lua as a reference, then it's fine too:

.. code-block:: cpp

	// :ok:
	lua["my_func"] = []() -> my_type* {
		static my_type mt;
		return &mt;
	};


Sol can detect ``nullptr``, so if you happen to return it there won't be any dangling because a ``sol::nil`` will be pushed.

.. code-block:: cpp

	struct my_type {
		void stuff () {}
	};

	sol::state lua;

	// BUT THIS IS STILL BAD DON'T DO IT AAAHHH BAD
	// return a unique_ptr still or something!
	lua["my_func"] = []() -> my_type* {
		return nullptr;
	};

	lua["my_func_2"] = [] () -> std::unique_ptr<my_type> {
		// default-constructs as a nullptr, 
		// gets pushed as nil to Lua 
		return std::unique_ptr<my_type>(); 
		// same happens for std::shared_ptr
	}

	// Acceptable, it will set 'something' to nil 
	// (and delete it on next GC if there's no more references)
	lua.set("something", nullptr);

	// Also fine
	lua["something_else"] = nullptr;
