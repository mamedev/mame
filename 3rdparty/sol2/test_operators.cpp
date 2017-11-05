#define SOL_CHECK_ARGUMENTS

#include <sol.hpp>
#include <catch.hpp>

TEST_CASE("operators/default", "test that generic equality operators and all sorts of equality tests can be used") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	struct T {};
	struct U {
		int a;
		U(int x = 20) : a(x) {}
		bool operator==(const U& r) {
			return a == r.a;
		}
	};
	struct V {
		int a;
		V(int x = 20) : a(x) {}
		bool operator==(const V& r) const {
			return a == r.a;
		}
	};
	lua.new_usertype<T>("T");
	lua.new_usertype<U>("U");
	lua.new_usertype<V>("V");

	T t1;
	T& t2 = t1;
	T t3;
	U u1;
	U u2{ 30 };
	U u3;
	U v1;
	U v2{ 30 };
	U v3;
	lua["t1"] = &t1;
	lua["t2"] = &t2;
	lua["t3"] = &t3;
	lua["u1"] = &u1;
	lua["u2"] = &u2;
	lua["u3"] = &u3;
	lua["v1"] = &v1;
	lua["v2"] = &v2;
	lua["v3"] = &v3;

	// Can only compare identity here
	REQUIRE_NOTHROW({
		lua.script("assert(t1 == t1)");
		lua.script("assert(t2 == t2)");
		lua.script("assert(t3 == t3)");
	});
	REQUIRE_NOTHROW({
		lua.script("assert(t1 == t2)");
		lua.script("assert(not (t1 == t3))");
		lua.script("assert(not (t2 == t3))");
	});
	// Object should compare equal to themselves
	// (and not invoke operator==; pointer test should be sufficient)
	REQUIRE_NOTHROW({
		lua.script("assert(u1 == u1)");
		lua.script("assert(u2 == u2)");
		lua.script("assert(u3 == u3)");
	});
	REQUIRE_NOTHROW({
		lua.script("assert(not (u1 == u2))");
		lua.script("assert(u1 == u3)");
		lua.script("assert(not (u2 == u3))");
	});
	// Object should compare equal to themselves
	// (and not invoke operator==; pointer test should be sufficient)
	REQUIRE_NOTHROW({
		lua.script("assert(v1 == v1)");
		lua.script("assert(v2 == v2)");
		lua.script("assert(v3 == v3)");
	});
	REQUIRE_NOTHROW({
		lua.script("assert(not (v1 == v2))");
		lua.script("assert(v1 == v3)");
		lua.script("assert(not (v2 == v3))");
	});
}