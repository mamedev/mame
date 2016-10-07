#define SOL_CHECK_ARGUMENTS

#include <sol.hpp>
#include <catch.hpp>

#include <iterator>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>

std::vector<int> test_table_return_one() {
	return{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
}

std::vector<std::pair<std::string, int>> test_table_return_two() {
	return{ { "one", 1 },{ "two", 2 },{ "three", 3 } };
}

std::map<std::string, std::string> test_table_return_three() {
	return{ { "name", "Rapptz" },{ "friend", "ThePhD" },{ "project", "sol" } };
}

TEST_CASE("containers/returns", "make sure that even references to vectors are being serialized as tables") {
	sol::state lua;
	std::vector<int> v{ 1, 2, 3 };
	lua.set_function("f", [&]() -> std::vector<int>& {
		return v;
	});
	lua.script("x = f()");
	sol::object x = lua["x"];
	sol::type xt = x.get_type();
	REQUIRE(xt == sol::type::userdata);
	sol::table t = x;
	bool matching;
	matching = t[1] == 1;
	REQUIRE(matching);
	matching = t[2] == 2;
	REQUIRE(matching);
	matching = t[3] == 3;
	REQUIRE(matching);
}

TEST_CASE("containers/vector_roundtrip", "make sure vectors can be round-tripped") {
	sol::state lua;
	std::vector<int> v{ 1, 2, 3 };
	lua.set_function("f", [&]() -> std::vector<int>& {
		return v;
	});
	lua.script("x = f()");
	std::vector<int> x = lua["x"];
	bool areequal = x == v;
	REQUIRE(areequal);
}

TEST_CASE("containers/list_roundtrip", "make sure lists can be round-tripped") {
	sol::state lua;
	std::list<int> v{ 1, 2, 3 };
	lua.set_function("f", [&]() -> std::list<int>& {
		return v;
	});
	lua.script("x = f()");
	std::list <int> x = lua["x"];
	bool areequal = x == v;
	REQUIRE(areequal);
}

TEST_CASE("containers/map_roundtrip", "make sure maps can be round-tripped") {
	sol::state lua;
	std::map<std::string, int> v{ { "a", 1 },{ "b", 2 },{ "c", 3 } };
	lua.set_function("f", [&]() -> std::map<std::string, int>& {
		return v;
	});
	lua.script("x = f()");
	std::map<std::string, int> x = lua["x"];
	bool areequal = x == v;
	REQUIRE(areequal);
}

TEST_CASE("containers/unordered_map_roundtrip", "make sure unordered_maps can be round-tripped") {
	sol::state lua;
	std::unordered_map<std::string, int> v{ { "a", 1 },{ "b", 2 },{ "c", 3 } };
	lua.set_function("f", [&]() -> std::unordered_map<std::string, int>& {
		return v;
	});
	lua.script("x = f()");
	std::unordered_map<std::string, int> x = lua["x"];
	bool areequal = x == v;
	REQUIRE(areequal);
}

TEST_CASE("containers/unordered_set_roundtrip", "make sure unordered_sets can be round-tripped") {
	sol::state lua;
	std::unordered_set<int> v{ 1, 2, 3 };
	lua.set_function("f", [&]() -> std::unordered_set<int>& {
		return v;
	});
	lua.script("x = f()");
	std::unordered_set<int> x = lua["x"];
	bool areequal = x == v;
	REQUIRE(areequal);
}

TEST_CASE("containers/set_roundtrip", "make sure sets can be round-tripped") {
	sol::state lua;
	std::set<int> v{ 1, 2, 3 };
	lua.set_function("f", [&]() -> std::set<int>& {
		return v;
	});
	lua.script("x = f()");
	std::set<int> x = lua["x"];
	bool areequal = x == v;
	REQUIRE(areequal);
}

TEST_CASE("containers/custom-usertype", "make sure container usertype metatables can be overridden") {
	typedef std::unordered_map<int, int> bark;
	
	sol::state lua;
	lua.open_libraries();
	lua.new_usertype<bark>("bark",
		"something", [](const bark& b) {
			INFO("It works: " << b.at(24));
		},
		"size", &bark::size,
		"at", sol::resolve<const int&>(&bark::at),
		"clear", &bark::clear
		);
	bark obj{ { 24, 50 } };
	lua.set("a", &obj);
	REQUIRE_NOTHROW(lua.script("assert(a:at(24) == 50)"));
	REQUIRE_NOTHROW(lua.script("a:something()"));
	lua.set("a", obj);
	REQUIRE_NOTHROW(lua.script("assert(a:at(24) == 50)"));
	REQUIRE_NOTHROW(lua.script("a:something()"));
}

TEST_CASE("containers/const-serialization-kvp", "make sure const keys / values are respected") {
	typedef std::map<int, const int> bark;

	sol::state lua;
	lua.open_libraries();
	bark obj{ { 24, 50 } };
	lua.set("a", &obj);
	REQUIRE_NOTHROW(lua.script("assert(a[24] == 50)"));
	REQUIRE_THROWS(lua.script("a[24] = 51"));
	REQUIRE_NOTHROW(lua.script("assert(a[24] == 50)"));
}

TEST_CASE("containers/basic-serialization", "make sure containers are turned into proper userdata and have basic hooks established") {
	typedef std::vector<int> woof;
	sol::state lua;
	lua.open_libraries();
	lua.set("b", woof{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30 });
	REQUIRE_NOTHROW(
	lua.script("for k = 1, #b do assert(k == b[k]) end");
	);
	woof w{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30 };
	lua.set("b", w);
	REQUIRE_NOTHROW(
		lua.script("for k = 1, #b do assert(k == b[k]) end");
	);
	lua.set("b", &w);
	REQUIRE_NOTHROW(
		lua.script("for k = 1, #b do assert(k == b[k]) end");
	);
	lua.set("b", std::ref(w));
	REQUIRE_NOTHROW(
		lua.script("for k = 1, #b do assert(k == b[k]) end");
	);
}

#if 0 // glibc is a fuccboi
TEST_CASE("containers/const-serialization", "make sure containers are turned into proper userdata and the basic hooks respect const-ness") {
	typedef std::vector<const int> woof;
	sol::state lua;
	lua.open_libraries();
	lua.set("b", woof{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30 });
	REQUIRE_NOTHROW(
		lua.script("for k, v in pairs(b) do assert(k == v) end");
	);
	REQUIRE_THROWS(lua.script("b[1] = 20"));
}
#endif // Fuck you, glibc

TEST_CASE("containers/table-serialization", "ensure types can be serialized as tables still") {
	typedef std::vector<int> woof;
	sol::state lua;
	lua.open_libraries();
	lua.set("b", sol::as_table(woof{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30 }));
	REQUIRE_NOTHROW(
		lua.script("for k, v in ipairs(b) do assert(k == v) end");
	);
	woof w{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30 };
	lua.set("b", sol::as_table(w));
	REQUIRE_NOTHROW(
	lua.script("for k, v in ipairs(b) do assert(k == v) end");
	);
	lua.set("b", sol::as_table(&w));
	REQUIRE_NOTHROW(
	lua.script("for k, v in ipairs(b) do assert(k == v) end");
	);
	lua.set("b", sol::as_table(std::ref(w)));
	REQUIRE_NOTHROW(
	lua.script("for k, v in ipairs(b) do assert(k == v) end");
	);
}

TEST_CASE("containers/const-correctness", "usertype metatable names should reasonably ignore const attributes") {
	struct Vec {
		int x, y, z;
	};

	sol::state lua;
	lua.open_libraries(sol::lib::base);
	lua.new_usertype<Vec>("Vec", "x", &Vec::x, "y", &Vec::y, "z", &Vec::z);

	Vec vec;
	vec.x = 1;
	vec.y = 2;
	vec.z = -3;

	std::vector<Vec> foo;
	foo.push_back(vec);

	std::vector<Vec const *> bar;
	bar.push_back(&vec);

	lua.script(R"(
func = function(vecs)
    for i = 1, #vecs do
		vec = vecs[i]
        print(i, ":", vec.x, vec.y, vec.z)
    end
end
)");

	REQUIRE_NOTHROW({
		lua["func"](foo);
		lua["func"](bar);
	});
}

TEST_CASE("containers/arbitrary-creation", "userdata and tables should be usable from standard containers") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);
	lua.set_function("test_one", test_table_return_one);
	lua.set_function("test_two", test_table_return_two);
	lua.set_function("test_three", test_table_return_three);

	REQUIRE_NOTHROW(lua.script("a = test_one()"));
	REQUIRE_NOTHROW(lua.script("b = test_two()"));
	REQUIRE_NOTHROW(lua.script("c = test_three()"));

	REQUIRE_NOTHROW(lua.script("assert(#a == 10, 'error')"));
	REQUIRE_NOTHROW(lua.script("assert(a[3] == 3, 'error')"));
	REQUIRE_NOTHROW(lua.script("assert(b.one == 1, 'error')"));
	REQUIRE_NOTHROW(lua.script("assert(b.three == 3, 'error')"));
	REQUIRE_NOTHROW(lua.script("assert(c.name == 'Rapptz', 'error')"));
	REQUIRE_NOTHROW(lua.script("assert(c.project == 'sol', 'error')"));

	sol::table a = lua.get<sol::table>("a");
	sol::table b = lua.get<sol::table>("b");
	sol::table c = lua.get<sol::table>("c");

	REQUIRE(a.size() == 10ULL);
	REQUIRE(a.get<int>(3) == 3);
	REQUIRE(b.get<int>("one") == 1);
	REQUIRE(b.get<int>("three") == 3);
	REQUIRE(c.get<std::string>("name") == "Rapptz");
	REQUIRE(c.get<std::string>("project") == "sol");
}

TEST_CASE("containers/usertype-transparency", "Make sure containers pass their arguments through transparently and push the results as references, not new values") {
	class A {
	public:
		int a;
		A(int b = 2) : a(b) {};

		void func() { }
	};

	struct B {

		B() {
			for (std::size_t i = 0; i < 20; ++i) {
				a_list.emplace_back(static_cast<int>(i));
			}
		}

		std::vector<A> a_list;
	};

	sol::state lua;
	lua.new_usertype<B>("B",
		"a_list", &B::a_list
		);

	lua.script(R"(
b = B.new()
a_ref = b.a_list[2]
)");

	B& b = lua["b"];
	A& a_ref = lua["a_ref"];
	REQUIRE(&b.a_list[1] == &a_ref);
	REQUIRE(b.a_list[1].a == a_ref.a);
}
