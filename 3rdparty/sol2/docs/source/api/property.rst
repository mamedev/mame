property
========

.. code-block:: cpp
	
	template <typename Read, typename Write>
	decltype(auto) property ( Read&& read_function, Write&& write_function );
	template <typename Read>
	decltype(auto) property ( Read&& read_function );
	template <typename Write>
	decltype(auto) property ( Write&& write_function );

These set of functions create a type which allows a setter and getter pair (or a single getter, or a single setter) to be used to create a variable that is either read-write, read-only, or write-only. When used during :doc:`usertype<usertype>` construction, it will create a variable that uses the setter/getter member function specified.

.. code-block:: cpp
	:caption: player.hpp
	:linenos:

	class Player {
	public:
		int get_hp() const {
			return hp;
		}

		void set_hp( int value ) {
			hp = value;
		}

		int get_max_hp() const {
			return hp;
		}

		void set_max_hp( int value ) {
			maxhp = value;
		}

	private:
		int hp = 50;
		int maxHp = 50;
	}

.. code-block:: cpp
	:caption: game.cpp
	:linenos:

	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.set("theplayer", Player());

	// Yes, you can register after you set a value and it will
	// connect up the usertype automatically
	lua.new_usertype<Player>( "Player",
		"hp", sol::property(&Player::get_hp, &Player::set_hp),
		"maxHp", sol::property(&Player::get_max_hp, &Player::set_max_hp)
	);


.. code-block:: lua
	:caption: game-snippet.lua

	-- variable syntax, calls functions
	theplayer.hp = 20
	print(theplayer.hp)