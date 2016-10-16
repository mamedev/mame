metatable_key
=============
A key for setting and getting an object's metatable
---------------------------------------------------

.. code-block:: cpp

	struct metatable_key_t {};
	const metatable_key_t metatable_key;

You can use this in conjunction with :doc:`sol::table<table>` to set/get a metatable. Lua metatables are powerful ways to override default behavior of objects for various kinds of operators, among other things. Here is an entirely complete example, showing getting and working with a :doc:`usertype<usertype>`'s metatable defined by Sol:

.. code-block:: cpp
	:caption: messing with metatables
	:linenos:

	#include <sol.hpp>

	int main () {

		struct bark {
			int operator()(int x) {
				return x;
			}
		};

		sol::state lua;
		lua.open_libraries(sol::lib::base);

		lua.new_usertype<bark>("bark",
			sol::meta_function::call_function, &bark::operator()
		);

		bark b;
		lua.set("b", &b);

		sol::table b_as_table = lua["b"];		
		sol::table b_metatable = b_as_table[sol::metatable_key];
		sol::function b_call = b_metatable["__call"];
		sol::function b_as_function = lua["b"];

		int result1 = b_as_function(1);
		int result2 = b_call(b, 1);
		// result1 == result2 == 1
	}

It's further possible to have a "Dynamic Getter" (`thanks to billw2012 and Nava2 for this example`_!):

.. code-block:: cpp
	:caption: One way to make dynamic properties (there are others!)
	:linenos:

	#include <sol.hpp>
	#include <unordered_map>

	struct PropertySet {
		sol::object get_property_lua(const char* name, sol::this_state s)
		{
			auto& var = props[name];
			return sol::make_object(s, var);
		}

		void set_property_lua(const char* name, sol::stack_object object)
		{
			props[name] = object.as<std::string>();
		}

		std::unordered_map<std::string, std::string> props;
	};

	struct DynamicObject {
		PropertySet& get_dynamic_props() {
			return dynamic_props;
		}

		PropertySet dynamic_props;
	};


	int main () {
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		lua.new_usertype<PropertySet>("PropertySet", 
			sol::meta_function::new_index, &PropertySet::set_property_lua,
			sol::meta_function::index, &PropertySet::get_property_lua
		);

		lua.new_usertype<DynamicObject>("DynamicObject", 
			"props", sol::property(&DynamicObject::get_dynamic_props)
		);

		lua.script(R"(
			obj = DynamicObject:new()
			obj.props.name = 'test name'
			print('name = ' .. obj.props.name)
		)");

		std::string name = lua["obj"]["props"]["name"];
		// name == "test name";
	}


You can even manipulate the ability to set and get items using metatable objects on a usertype or similar:

.. code-block:: cpp
	:caption: messing with metatables - vector type
	:linenos:

	#include <sol.hpp>

	class vector {
	public:
		double data[3];

		vector() : data{ 0,0,0 } {}

		double& operator[](int i)
		{
			return data[i];
		}


		static double my_index(vector& v, int i)
		{
			return v[i];
		}

		static void my_new_index(vector& v, int i, double x)
		{
			v[i] = x;
		}
	};

	int main () {
		sol::state lua;
		lua.open_libraries(sol::lib::base);
		lua.new_usertype<vector>("vector", sol::constructors<sol::types<>>(),
			sol::meta_function::index, &vector::my_index,
			sol::meta_function::new_index, &vector::my_new_index);
		lua.script("v = vector.new()\n"
			"print(v[1])\n"
			"v[2] = 3\n"
			"print(v[2])\n"
		);

		vector& v = lua["v"];
		// v[0] == 0.0;
		// v[1] == 0.0;
		// v[2] == 3.0;
	}


.. _thanks to billw2012 and Nava2 for this example: https://github.com/ThePhD/sol2/issues/71#issuecomment-225402055