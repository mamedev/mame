#include <sol.hpp>

#include <iostream>

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

	void boost() {
		speed += 10;
	}

	bool shoot() {
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

int main() {
	std::cout << "=== usertype_advanced example ===" << std::endl;
	sol::state lua;

	lua.open_libraries(sol::lib::base);

	// note that you can set a 
	// userdata before you register a usertype,
	// and it will still carry 
	// the right metatable if you register it later

	// set a variable "p2" of type "player" with 0 ammo
	lua["p2"] = player(0);

	// make usertype metatable
	lua.new_usertype<player>("player",

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
		"bullets", sol::readonly(&player::bullets)
		);

	std::string player_script = R"(
-- call single argument integer constructor
p1 = player.new(2)

-- p2 is still here from being 
-- set with lua["p2"] = player(0); below
local p2shoots = p2:shoot()
assert(not p2shoots)
-- had 0 ammo
	
-- set variable property setter
p1.hp = 545
-- get variable through property getter
print(p1.hp)
assert(p1.hp == 545)

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
)";

	// Uncomment and use the file to try that out, too!
	// Make sure it's in the local directory of the executable after you build, or adjust the filename path
	// Or whatever else you like!
	//lua.script_file("player_script.lua");
	lua.script(player_script);
	std::cout << std::endl;
}
