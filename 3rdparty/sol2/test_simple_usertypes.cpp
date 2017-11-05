#define SOL_CHECK_ARGUMENTS

#include <sol.hpp>
#include <catch.hpp>

#include <iostream>
#include <list>
#include <memory>
#include <mutex>

TEST_CASE("usertype/simple-usertypes", "Ensure that simple usertypes properly work here") {
	struct marker {
		bool value = false;
	};
	struct bark {
		int var = 50;
		marker mark;

		void fun() {
			var = 51;
		}

		int get() const {
			return var;
		}

		int set(int x) {
			var = x;
			return var;
		}

		std::string special() const {
			return mark.value ? "woof" : "pantpant";
		}

		const marker& the_marker() const {
			return mark;
		}
	};

	sol::state lua;
	lua.new_simple_usertype<bark>("bark",
		"fun", &bark::fun,
		"get", &bark::get,
		"var", sol::as_function( &bark::var ),
		"the_marker", sol::as_function(&bark::the_marker),
		"x", sol::overload(&bark::get),
		"y", sol::overload(&bark::set),
		"z", sol::overload(&bark::get, &bark::set)
	);

	lua.script("b = bark.new()");
	bark& b = lua["b"];

	lua.script("b:fun()");
	int var = b.var;
	REQUIRE(var == 51);

	lua.script("b:var(20)");
	lua.script("v = b:var()");
	int v = lua["v"];
	REQUIRE(v == 20);
	REQUIRE(b.var == 20);

	lua.script("m = b:the_marker()");
	marker& m = lua["m"];
	REQUIRE_FALSE(b.mark.value);
	REQUIRE_FALSE(m.value);
	m.value = true;
	REQUIRE(&b.mark == &m);
	REQUIRE(b.mark.value);

	sol::table barktable = lua["bark"];
	barktable["special"] = &bark::special;

	lua.script("s = b:special()");
	std::string s = lua["s"];
	REQUIRE(s == "woof");

	lua.script("b:y(24)");
	lua.script("x = b:x()");
	int x = lua["x"];
	REQUIRE(x == 24);

	lua.script("z = b:z(b:z() + 5)");
	int z = lua["z"];
	REQUIRE(z == 29);
}

TEST_CASE("usertype/simple-usertypes-constructors", "Ensure that calls with specific arguments work") {
	struct marker {
		bool value = false;
	};
	struct bark {
		int var = 50;
		marker mark;

		bark() {}
		bark(int v) : var(v) {}

		void fun() {
			var = 51;
		}

		int get() const {
			return var;
		}

		int set(int x) {
			var = x;
			return var;
		}

		std::string special() const {
			return mark.value ? "woof" : "pantpant";
		}

		const marker& the_marker() const {
			return mark;
		}
	};

	sol::state lua;
	lua.new_simple_usertype<bark>("bark",
		sol::constructors<sol::types<>, sol::types<int>>(),
		"fun", sol::protect( &bark::fun ),
		"get", &bark::get,
		"var", sol::as_function( &bark::var ),
		"the_marker", &bark::the_marker,
		"x", sol::overload(&bark::get),
		"y", sol::overload(&bark::set),
		"z", sol::overload(&bark::get, &bark::set)
	);

	lua.script("bx = bark.new(760)");
	bark& bx = lua["bx"];
	REQUIRE(bx.var == 760);

	lua.script("b = bark.new()");
	bark& b = lua["b"];

	lua.script("b:fun()");
	int var = b.var;
	REQUIRE(var == 51);

	lua.script("b:var(20)");
	lua.script("v = b:var()");
	int v = lua["v"];
	REQUIRE(v == 20);

	lua.script("m = b:the_marker()");
	marker& m = lua["m"];
	REQUIRE_FALSE(b.mark.value);
	REQUIRE_FALSE(m.value);
	m.value = true;
	REQUIRE(&b.mark == &m);
	REQUIRE(b.mark.value);

	sol::table barktable = lua["bark"];
	barktable["special"] = &bark::special;

	lua.script("s = b:special()");
	std::string s = lua["s"];
	REQUIRE(s == "woof");

	lua.script("b:y(24)");
	lua.script("x = b:x()");
	int x = lua["x"];
	REQUIRE(x == 24);

	lua.script("z = b:z(b:z() + 5)");
	int z = lua["z"];
	REQUIRE(z == 29);
}

TEST_CASE("usertype/simple-shared-ptr-regression", "simple usertype metatables should not screw over unique usertype metatables") {
	static int created = 0;
	static int destroyed = 0;
	struct test {
		test() {
			++created;
		}

		~test() {
			++destroyed;
		}
	};
	{
		std::list<std::shared_ptr<test>> tests;
		sol::state lua;
		lua.open_libraries();

		lua.new_simple_usertype<test>("test",
			"create", [&]() -> std::shared_ptr<test> {
			tests.push_back(std::make_shared<test>());
			return tests.back();
		}
		);
		REQUIRE(created == 0);
		REQUIRE(destroyed == 0);
		lua.script("x = test.create()");
		REQUIRE(created == 1);
		REQUIRE(destroyed == 0);
		REQUIRE_FALSE(tests.empty());
		std::shared_ptr<test>& x = lua["x"];
		std::size_t xuse = x.use_count();
		std::size_t tuse = tests.back().use_count();
		REQUIRE(xuse == tuse);
	}
	REQUIRE(created == 1);
	REQUIRE(destroyed == 1);
}

TEST_CASE("usertype/simple-vars", "simple usertype vars can bind various values (no ref)") {
	int muh_variable = 10;
	int through_variable = 25;

	sol::state lua;
	lua.open_libraries();

	struct test {};
	lua.new_simple_usertype<test>("test",
		"straight", sol::var(2),
		"global", sol::var(muh_variable),
		"global2", sol::var(through_variable),
		"global3", sol::var(std::ref(through_variable))
	);
	
	through_variable = 20;

	lua.script(R"(
print(test.straight)
s = test.straight
print(test.global)
g = test.global
print(test.global2)
g2 = test.global2
print(test.global3)
g3 = test.global3
)");

	int s = lua["s"];
	int g = lua["g"];
	int g2 = lua["g2"];
	int g3 = lua["g3"];
	REQUIRE(s == 2);
	REQUIRE(g == 10);
	REQUIRE(g2 == 25);
	REQUIRE(g3 == 20);
}

TEST_CASE("usertypes/simple-variable-control", "test to see if usertypes respond to inheritance and variable controls") {
	class A {
	public:
		virtual void a() { throw std::runtime_error("entered base pure virtual implementation"); };
	};

	class B : public A {
	public:
		virtual void a() override { }
	};

	class sA {
	public:
		virtual void a() { throw std::runtime_error("entered base pure virtual implementation"); };
	};

	class sB : public sA {
	public:
		virtual void a() override { }
	};

	struct sV {
		int a = 10;
		int b = 20;

		int get_b() const {
			return b + 2;
		}

		void set_b(int value) {
			b = value;
		}
	};

	struct sW : sV {};

	sol::state lua;
	lua.open_libraries();

	lua.new_usertype<A>("A", "a", &A::a);
	lua.new_usertype<B>("B", sol::base_classes, sol::bases<A>());
	lua.new_simple_usertype<sA>("sA", "a", &sA::a);
	lua.new_simple_usertype<sB>("sB", sol::base_classes, sol::bases<sA>());
	lua.new_simple_usertype<sV>("sV", "a", &sV::a, "b", &sV::b, "pb", sol::property(&sV::get_b, &sV::set_b));
	lua.new_simple_usertype<sW>("sW", sol::base_classes, sol::bases<sV>());

	B b;
	lua.set("b", &b);
	lua.script("b:a()");

	sB sb;
	lua.set("sb", &sb);
	lua.script("sb:a()");

	sV sv;
	lua.set("sv", &sv);
	lua.script("print(sv.b)assert(sv.b == 20)");

	sW sw;
	lua.set("sw", &sw);
	lua.script("print(sw.a)assert(sw.a == 10)");
	lua.script("print(sw.b)assert(sw.b == 20)");
	lua.script("print(sw.pb)assert(sw.pb == 22)");
	lua.script("sw.a = 11");
	lua.script("sw.b = 21");
	lua.script("print(sw.a)assert(sw.a == 11)");
	lua.script("print(sw.b)assert(sw.b == 21)");
	lua.script("print(sw.pb)assert(sw.pb == 23)");
	lua.script("sw.pb = 25");
	lua.script("print(sw.b)assert(sw.b == 25)");
	lua.script("print(sw.pb)assert(sw.pb == 27)");
}

TEST_CASE("usertype/simple-factory-constructor-overload-usage", "simple usertypes should probably invoke types") {
	class A {
	public:
		virtual void a() { throw std::runtime_error("entered base pure virtual implementation"); };
	};

	class B : public A {
	public:
		int bvar = 24;
		virtual void a() override { }
		void f() {}
	};

	sol::state lua;
	lua.open_libraries();
	sol::constructors<sol::types<>, sol::types<const B&>> c;
	lua.new_simple_usertype<B>("B",
		sol::call_constructor, c,
		"new", sol::factories([]() { return B(); }),
		"new2", sol::initializers([](B& mem) { new(&mem)B(); }, [](B& mem, int v) { new(&mem)B(); mem.bvar = v; }),
		"f", sol::as_function(&B::bvar),
		"g", sol::overload([](B&) { return 2; }, [](B&, int v) { return v; })
	);

	lua.script("b = B()");
	lua.script("b2 = B.new()");
	lua.script("b3 = B.new2()");
	lua.script("b4 = B.new2(11)");

	lua.script("x = b:f()");
	lua.script("x2 = b2:f()");
	lua.script("x3 = b3:f()");
	lua.script("x4 = b4:f()");
	int x = lua["x"];
	int x2 = lua["x2"];
	int x3 = lua["x3"];
	int x4 = lua["x4"];
	REQUIRE(x == 24);
	REQUIRE(x2 == 24);
	REQUIRE(x3 == 24);
	REQUIRE(x4 == 11);

	lua.script("y = b:g()");
	lua.script("y2 = b2:g(3)");
	lua.script("y3 = b3:g()");
	lua.script("y4 = b4:g(3)");
	int y = lua["y"];
	int y2 = lua["y2"];
	int y3 = lua["y3"];
	int y4 = lua["y4"];
	REQUIRE(y == 2);
	REQUIRE(y2 == 3);
	REQUIRE(y3 == 2);
	REQUIRE(y4 == 3);
}

TEST_CASE("usertype/simple-runtime-append", "allow extra functions to be appended at runtime directly to the metatable itself") {
	class A {
	};

	class B : public A {
	};

	sol::state lua;
	lua.new_simple_usertype<A>("A");
	lua.new_simple_usertype<B>("B", sol::base_classes, sol::bases<A>());
	lua.set("b", std::make_unique<B>());
	lua["A"]["method"] = []() { return 200; };
	lua["B"]["method2"] = [](B&) { return 100; };
	lua.script("x = b.method()");
	lua.script("y = b:method()");

	int x = lua["x"];
	int y = lua["y"];
	REQUIRE(x == 200);
	REQUIRE(y == 200);
	
	lua.script("z = b.method2(b)");
	lua.script("w = b:method2()");
	int z = lua["z"];
	int w = lua["w"];
	REQUIRE(z == 100);
	REQUIRE(w == 100);
}

TEST_CASE("usertype/simple-destruction-test", "make sure usertypes are properly destructed and don't double-delete memory or segfault") {
	sol::state lua;

	class CrashClass {
	public:
		CrashClass() {
		}

		~CrashClass() {
			a = 10; // This will cause a crash.
		}

	private:
		int a;
	};

	lua.new_simple_usertype<CrashClass>("CrashClass",
		sol::call_constructor, sol::constructors<sol::types<>>()
		);

	lua.script(R"(
		function testCrash()
			local x = CrashClass()
			end
		)");

	for (int i = 0; i < 1000; ++i) {
		lua["testCrash"]();
	}
}

TEST_CASE("usertype/simple-table-append", "Ensure that appending to the meta table also affects the internal function table for pointers as well") {
	struct A {
		int func() {
			return 5000;
		}
	};

	sol::state lua;
	lua.open_libraries();

	lua.new_simple_usertype<A>("A");
	sol::table table = lua["A"];
	table["func"] = &A::func;
	A a;
	lua.set("a", &a);
	lua.set("pa", &a);
	lua.set("ua", std::make_unique<A>());
	REQUIRE_NOTHROW(
		lua.script("assert(a:func() == 5000)");
		lua.script("assert(pa:func() == 5000)");
		lua.script("assert(ua:func() == 5000)");
	);
}
