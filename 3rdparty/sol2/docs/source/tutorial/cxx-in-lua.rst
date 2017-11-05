C++ in Lua
==========

Using user defined types ("usertype"s, or just "udt"s) is simple with Sol. If you don't call any member variables or functions, then you don't even have to 'register' the usertype at all: just pass it through. But if you want variables and functions on your usertype inside of Lua, you need to register it. We're going to give a short example here that includes a bunch of information on how to work with things.

Take this ``player`` struct in C++ in a header file:

.. code-block:: cpp
	:caption: test_player.hpp

	struct player {
	public:
		int bullets;
		int speed;

		player() 
		: player(3, 100) {

		}

		player(int ammo) 
		: player(ammo, 100) {

		}

		player(int ammo, int hitpoints) 
		: bullets(ammo), hp(hitpoints) {

		}

		void boost () {
			speed += 10;
		}

		bool shoot () {
			if (bullets < 1)
				return false;
			--bullets;
			return true;
		}

		void set_hp(int value) {
			hp = value;
		}

		int get_hp() const {
			return hp;
		}

	private:
		int hp;
	};


It's a fairly minimal class, but we don't want to have to rewrite this with metatables in Lua. We want this to be part of Lua easily. The following is the Lua code that we'd like to have work properly:

.. code-block:: lua
	:caption: player_script.lua
	
	-- call single argument integer constructor
	p1 = player.new(2)

	-- p2 is still here from being 
	-- set with lua["p2"] = player(0);
	-- in cpp file
	local p2shoots = p2:shoot()
	assert(not p2shoots)
	-- had 0 ammo
	
	-- set variable property setter
	p1.hp = 545;
	-- get variable through property getter
	print(p1.hp);

	local did_shoot_1 = p1:shoot()
	print(did_shoot_1)
	print(p1.bullets)
	local did_shoot_2 = p1:shoot()
	print(did_shoot_2)
	print(p1.bullets)
	local did_shoot_3 = p1:shoot()
	print(did_shoot_3)
	
	-- can read
	print(p1.bullets)
	-- would error: is a readonly variable, cannot write
	-- p1.bullets = 20

	p1:boost()

To do this, you bind things using the ``new_usertype`` and ``set_usertype`` methods as shown below. These methods are on both :doc:`table<../api/table>` and :doc:`state(_view)<../api/state>`, but we're going to just use it on ``state``:

.. code-block:: cpp
	:caption: player_script.cpp

	#include <sol.hpp>

	int main () {
		sol::state lua;

		// note that you can set a 
		// userdata before you register a usertype,
		// and it will still carry 
		// the right metatable if you register it later
		
		// set a variable "p2" of type "player" with 0 ammo
		lua["p2"] = player(0);

		// make usertype metatable
		lua.new_usertype<player>( "player",
			
			// 3 constructors
			sol::constructors<sol::types<>, sol::types<int>, sol::types<int, int>>(),
			
			// typical member function that returns a variable
			"shoot", &player::shoot,
			// typical member function
			"boost", &player::boost,
			
			// gets or set the value using member variable syntax
			"hp", sol::property(&player::get_hp, &player::set_hp),
			
			// read and write variable
			"speed", &player::speed,
			// can only read from, not write to
			"bullets", sol::readonly( &player::bullets )
		);

		lua.script_file("player_script.lua");
	}

That script should run fine now, and you can observe and play around with the values. Even more stuff :doc:`you can do<../api/usertype>` is described elsewhere, like initializer functions (private constructors / destructors support), "static" functions callable with ``name.my_function( ... )``, and overloaded member functions. You can even bind global variables (even by reference with ``std::ref``) with ``sol::var``. There's a lot to try out!

This is a powerful way to allow reuse of C++ code from Lua beyond just registering functions, and should get you on your way to having more complex classes and data structures! In the case that you need more customization than just usertypes, however, you can customize Sol to behave more fit to your desires by using the desired :doc:`customization and extension structures<customization>`.

You can check out this code and more complicated code at the `examples directory`_ by looking at the ``usertype_``-prefixed examples.

.. _examples directory: https://github.com/ThePhD/sol2/tree/develop/examples