#define SOL_CHECK_ARGUMENTS

#include <sol.hpp>
#include <catch.hpp>

#include <iostream>

TEST_CASE("inheritance/basic", "test that metatables are properly inherited") {
	struct A {
		int a = 5;
	};

	struct B {
		int b() {
			return 10;
		}
	};

	struct C : B, A {
		double c = 2.4;
	};

	struct D : C {
		bool d() const {
			return true;
		}
	};

	sol::state lua;
	lua.new_usertype<A>("A",
		"a", &A::a
		);
	lua.new_usertype<B>("B",
		"b", &B::b
		);
	lua.new_usertype<C>("C",
		"c", &C::c,
		sol::base_classes, sol::bases<B, A>()
		);
	lua.new_usertype<D>("D",
		"d", &D::d,
		sol::base_classes, sol::bases<C, B, A>()
		);

	lua.script("obj = D.new()");
	lua.script("d = obj:d()");
	bool d = lua["d"];
	lua.script("c = obj.c");
	double c = lua["c"];
	lua.script("b = obj:b()");
	int b = lua["b"];
	lua.script("a = obj.a");
	int a = lua["a"];

	REQUIRE(d);
	REQUIRE(c == 2.4);
	REQUIRE(b == 10);
	REQUIRE(a == 5);
}

TEST_CASE("inheritance/multi-base", "test that multiple bases all work and overloading for constructors works with them") {
	class TestClass00 {
	public:
		int Thing() const { return 123; }
	};

	class TestClass01 : public TestClass00 {
	public:
		TestClass01() : a(1) {}
		TestClass01(const TestClass00& other) : a(other.Thing()) {}

		int a;
	};

	class TestClass02 : public TestClass01 {
	public:
		TestClass02() : b(2) {}
		TestClass02(const TestClass01& other) : b(other.a) {}
		TestClass02(const TestClass00& other) : b(other.Thing()) {}

		int b;
	};

	class TestClass03 : public TestClass02 {
	public:
		TestClass03() : c(2) {}
		TestClass03(const TestClass02& other) : c(other.b) {}
		TestClass03(const TestClass01& other) : c(other.a) {}
		TestClass03(const TestClass00& other) : c(other.Thing()) {}

		int c;
	};

	sol::state lua;

	sol::usertype<TestClass00> s_TestUsertype00(
		sol::call_constructor, sol::constructors<sol::types<>>(),
		"Thing", &TestClass00::Thing
	);

	lua.set_usertype("TestClass00", s_TestUsertype00);

	sol::usertype<TestClass01> s_TestUsertype01(
		sol::call_constructor, sol::constructors<sol::types<>, sol::types<const TestClass00&>>(),
		sol::base_classes, sol::bases<TestClass00>(),
		"a", &TestClass01::a
	);

	lua.set_usertype("TestClass01", s_TestUsertype01);

	sol::usertype<TestClass02> s_TestUsertype02(
		sol::call_constructor, sol::constructors<sol::types<>, sol::types<const TestClass01&>, sol::types<const TestClass00&>>(),
		sol::base_classes, sol::bases<TestClass01, TestClass00>(),
		"b", &TestClass02::b
	);

	lua.set_usertype("TestClass02", s_TestUsertype02);

	sol::usertype<TestClass03> s_TestUsertype03(
		sol::call_constructor, sol::constructors<sol::types<>, sol::types<const TestClass02&>, sol::types<const TestClass01&>, sol::types<const TestClass00&>>(),
		sol::base_classes, sol::bases<TestClass02, TestClass01, TestClass00>(),
		"c", &TestClass03::c
	);

	lua.set_usertype("TestClass03", s_TestUsertype03);

	lua.script(R"(
tc0 = TestClass00()
)");

	lua.script(R"(
tc2 = TestClass02(tc0)
)");

	lua.script(R"(
tc1 = TestClass01()
)");

	lua.script(R"(
tc3 = TestClass03(tc1)
)");

	TestClass00& tc0 = lua["tc0"];
	TestClass01& tc1 = lua["tc1"];
	TestClass02& tc2 = lua["tc2"];
	TestClass03& tc3 = lua["tc3"];
	REQUIRE(tc1.a == 1);
	REQUIRE(tc2.a == 1);
	REQUIRE(tc2.b == 123);
	REQUIRE(tc3.a == 1);
	REQUIRE(tc3.b == 2);
	REQUIRE(tc3.c == 1);
}

TEST_CASE("inheritance/simple-multi-base", "test that multiple bases all work and overloading for constructors works with them") {
	class TestClass00 {
	public:
		int Thing() const { return 123; }
	};

	class TestClass01 : public TestClass00 {
	public:
		TestClass01() : a(1) {}
		TestClass01(const TestClass00& other) : a(other.Thing()) {}

		int a;
	};

	class TestClass02 : public TestClass01 {
	public:
		TestClass02() : b(2) {}
		TestClass02(const TestClass01& other) : b(other.a) {}
		TestClass02(const TestClass00& other) : b(other.Thing()) {}

		int b;
	};

	class TestClass03 : public TestClass02 {
	public:
		TestClass03() : c(2) {}
		TestClass03(const TestClass02& other) : c(other.b) {}
		TestClass03(const TestClass01& other) : c(other.a) {}
		TestClass03(const TestClass00& other) : c(other.Thing()) {}

		int c;
	};

	sol::state lua;

	sol::simple_usertype<TestClass00> s_TestUsertype00( lua,
		sol::call_constructor, sol::constructors<sol::types<>>(),
		"Thing", &TestClass00::Thing
	);

	lua.set_usertype("TestClass00", s_TestUsertype00);

	sol::simple_usertype<TestClass01> s_TestUsertype01( lua,
		sol::call_constructor, sol::constructors<sol::types<>, sol::types<const TestClass00&>>(),
		sol::base_classes, sol::bases<TestClass00>(),
		"a", &TestClass01::a
	);

	lua.set_usertype("TestClass01", s_TestUsertype01);

	sol::simple_usertype<TestClass02> s_TestUsertype02( lua,
		sol::call_constructor, sol::constructors<sol::types<>, sol::types<const TestClass01&>, sol::types<const TestClass00&>>(),
		sol::base_classes, sol::bases<TestClass01, TestClass00>(),
		"b", &TestClass02::b
	);

	lua.set_usertype("TestClass02", s_TestUsertype02);

	sol::simple_usertype<TestClass03> s_TestUsertype03( lua,
		sol::call_constructor, sol::constructors<sol::types<>, sol::types<const TestClass02&>, sol::types<const TestClass01&>, sol::types<const TestClass00&>>(),
		sol::base_classes, sol::bases<TestClass02, TestClass01, TestClass00>(),
		"c", &TestClass03::c
	);

	lua.set_usertype("TestClass03", s_TestUsertype03);

	lua.script(R"(
tc0 = TestClass00()
)");

	lua.script(R"(
tc2 = TestClass02(tc0)
)");

	lua.script(R"(
tc1 = TestClass01()
)");

	lua.script(R"(
tc3 = TestClass03(tc1)
)");

	TestClass00& tc0 = lua["tc0"];
	TestClass01& tc1 = lua["tc1"];
	TestClass02& tc2 = lua["tc2"];
	TestClass03& tc3 = lua["tc3"];
	REQUIRE(tc1.a == 1);
	REQUIRE(tc2.a == 1);
	REQUIRE(tc2.b == 123);
	REQUIRE(tc3.a == 1);
	REQUIRE(tc3.b == 2);
	REQUIRE(tc3.c == 1);
}
