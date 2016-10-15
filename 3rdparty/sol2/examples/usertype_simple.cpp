#define SOL_CHECK_ARGUMENTS
#include <sol.hpp>
#include <memory>
#include <iostream>
#include <cassert>

class generator {
private:
	int data = 2;

public:
	int get_data() const { return data; }
	void set_data(int value) { data = value % 10; }

	std::vector<int> generate_list() {
		return { data, data * 2, data * 3, data * 4, data * 5 };
	}
};

struct my_data {
	int first = 4;
	int second = 8;
	int third = 12;
};

int main() {
	std::cout << "=== usertype_simple example ===" << std::endl;

	sol::state lua;
	lua.open_libraries();

	// simple_usertype acts and behaves like
	// a regular usertype
	lua.new_simple_usertype<my_data>("my_data",
		"first", &my_data::first,
		"second", &my_data::second,
		"third", &my_data::third
	);

	{
		// But, simple_usertype also has a `set` function
		// where you can append things to the 
		// method listing after doing `create_simple_usertype`.
		auto generator_registration = lua.create_simple_usertype<generator>();
		generator_registration.set("data", sol::property(&generator::get_data, &generator::set_data));
		// you MUST set the usertype after
		// creating it
		lua.set_usertype("generator", generator_registration);
	}

	// Can update a simple_usertype at runtime, after registration
	lua["generator"]["generate_list"] = [](generator& self) { return self.generate_list(); };
	// can set 'static methods' (no self) as well
	lua["generator"]["get_num"] = []() { return 100; };
	
	// Mix it all together!
	lua.script(R"(
mdata = my_data.new()

local g = generator.new()
g.data = mdata.first
list1 = g:generate_list()
g.data = mdata.second
list2 = g:generate_list()
g.data = mdata.third
list3 = g:generate_list()

print("From lua: ")
for i = 1, #list1 do
	print("\tlist1[" .. i .. "] = " .. list1[i])
end
for i = 1, #list2 do
	print("\tlist2[" .. i .. "] = " .. list2[i])
end
for i = 1, #list3 do
	print("\tlist3[" .. i .. "] = " .. list3[i])
end

)");
	my_data& mdata = lua["mdata"];
	std::vector<int>& list1 = lua["list1"];
	std::vector<int>& list2 = lua["list2"];
	std::vector<int>& list3 = lua["list3"];
	assert(list1.size() == 5);
	assert(list2.size() == 5);
	assert(list3.size() == 5);
	for (int i = 1; i <= 5; ++i) {
		assert(list1[i - 1] == (mdata.first % 10) * i);
		assert(list2[i - 1] == (mdata.second % 10) * i);
		assert(list3[i - 1] == (mdata.third % 10) * i);
	}
	
	std::cout << std::endl;
	return 0;
}