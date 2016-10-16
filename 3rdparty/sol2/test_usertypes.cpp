#define SOL_CHECK_ARGUMENTS

#include <sol.hpp>
#include <catch.hpp>

#include <iostream>
#include <list>
#include <memory>
#include <mutex>

struct vars {
	vars() {

	}

	int boop = 0;

	~vars() {

	}
};

struct fuser {
	int x;
	fuser() : x(0) {}

	fuser(int x) : x(x) {}

	int add(int y) {
		return x + y;
	}

	int add2(int y) {
		return x + y + 2;
	}
};

namespace crapola {
	struct fuser {
		int x;
		fuser() : x(0) {}
		fuser(int x) : x(x) {}
		fuser(int x, int x2) : x(x * x2) {}

		int add(int y) {
			return x + y;
		}
		int add2(int y) {
			return x + y + 2;
		}
	};
} // crapola

class Base {
public:
	Base(int a_num) : m_num(a_num) { }

	int get_num() {
		return m_num;
	}

protected:
	int m_num;
};

class Derived : public Base {
public:
	Derived(int a_num) : Base(a_num) { }

	int get_num_10() {
		return 10 * m_num;
	}
};

class abstract_A {
public:
	virtual void a() = 0;
};

class abstract_B : public abstract_A {
public:
	virtual void a() override {
		INFO("overridden a() in B : public A - BARK");
	}
};

struct Vec {
	float x, y, z;
	Vec(float x, float y, float z) : x{ x }, y{ y }, z{ z } {}
	float length() {
		return sqrtf(x*x + y*y + z*z);
	}
	Vec normalized() {
		float invS = 1 / length();
		return{ x * invS, y * invS, z * invS };
	}
};

struct giver {
	int a = 0;

	giver() {

	}

	void gief() {
		a = 1;
	}

	static void stuff() {

	}

	static void gief_stuff(giver& t, int a) {
		t.a = a;
	}

	~giver() {

	}

};

struct factory_test {
private:
	factory_test() { a = true_a; }
	~factory_test() { a = 0; }
public:
	static int num_saved;
	static int num_killed;

	struct deleter {
		void operator()(factory_test* f) {
			f->~factory_test();
		}
	};

	static const int true_a;
	int a;

	static std::unique_ptr<factory_test, deleter> make() {
		return std::unique_ptr<factory_test, deleter>(new factory_test(), deleter());
	}

	static void save(factory_test& f) {
		new(&f)factory_test();
		++num_saved;
	}

	static void kill(factory_test& f) {
		f.~factory_test();
		++num_killed;
	}
};

int factory_test::num_saved = 0;
int factory_test::num_killed = 0;
const int factory_test::true_a = 156;

bool something() {
	return true;
}

struct thing {
	int v = 100;

	thing() {}
	thing(int x) : v(x) {}
};

struct self_test {
	int bark;

	self_test() : bark(100) {

	}

	void g(const std::string& str) {
		std::cout << str << '\n';
		bark += 1;
	}

	void f(const self_test& t) {
		std::cout << "got test" << '\n';
		if (t.bark != bark)
			throw sol::error("bark values are not the same for self_test f function");
		if (&t != this)
			throw sol::error("call does not reference self for self_test f function");
	}
};

struct ext_getset {

	int bark = 24;
	const int meow = 56;

	ext_getset() = default;
	ext_getset(int v) : bark(v) {}
	ext_getset(ext_getset&&) = default;
	ext_getset(const ext_getset&) = delete;
	ext_getset& operator=(ext_getset&&) = default;
	ext_getset& operator=(const ext_getset&) = delete;
	~ext_getset() {

	}

	std::string x() {
		return "bark bark bark";
	}

	int x2(std::string x) {
		return static_cast<int>(x.length());
	}

	void set(sol::variadic_args, sol::this_state, int x) {
		bark = x;
	}

	int get(sol::this_state, sol::variadic_args) {
		return bark;
	}

	static void s_set(int) {

	}

	static int s_get(int x) {
		return x + 20;
	}

};

template <typename T>
void des(T& e) {
	e.~T();
}

TEST_CASE("usertype/usertype", "Show that we can create classes from usertype and use them") {
	sol::state lua;

	sol::usertype<fuser> lc{ "add", &fuser::add, "add2", &fuser::add2 };
	lua.set_usertype(lc);

	lua.script("a = fuser:new()\n"
		"b = a:add(1)\n"
		"c = a:add2(1)\n");

	sol::object a = lua.get<sol::object>("a");
	sol::object b = lua.get<sol::object>("b");
	sol::object c = lua.get<sol::object>("c");
	REQUIRE((a.is<sol::userdata_value>()));
	auto atype = a.get_type();
	auto btype = b.get_type();
	auto ctype = c.get_type();
	REQUIRE((atype == sol::type::userdata));
	REQUIRE((btype == sol::type::number));
	REQUIRE((ctype == sol::type::number));
	int bresult = b.as<int>();
	int cresult = c.as<int>();
	REQUIRE(bresult == 1);
	REQUIRE(cresult == 3);
}

TEST_CASE("usertype/usertype-constructors", "Show that we can create classes from usertype and use them with multiple constructors") {

	sol::state lua;

	sol::constructors<sol::types<>, sol::types<int>, sol::types<int, int>> con;
	sol::usertype<crapola::fuser> lc(con, "add", &crapola::fuser::add, "add2", &crapola::fuser::add2);
	lua.set_usertype(lc);

	lua.script(
		"a = fuser.new(2)\n"
		"u = a:add(1)\n"
		"v = a:add2(1)\n"
		"b = fuser:new()\n"
		"w = b:add(1)\n"
		"x = b:add2(1)\n"
		"c = fuser.new(2, 3)\n"
		"y = c:add(1)\n"
		"z = c:add2(1)\n");
	sol::object a = lua.get<sol::object>("a");
	auto atype = a.get_type();
	REQUIRE((atype == sol::type::userdata));
	sol::object u = lua.get<sol::object>("u");
	sol::object v = lua.get<sol::object>("v");
	REQUIRE((u.as<int>() == 3));
	REQUIRE((v.as<int>() == 5));

	sol::object b = lua.get<sol::object>("b");
	auto btype = b.get_type();
	REQUIRE((btype == sol::type::userdata));
	sol::object w = lua.get<sol::object>("w");
	sol::object x = lua.get<sol::object>("x");
	REQUIRE((w.as<int>() == 1));
	REQUIRE((x.as<int>() == 3));

	sol::object c = lua.get<sol::object>("c");
	auto ctype = c.get_type();
	REQUIRE((ctype == sol::type::userdata));
	sol::object y = lua.get<sol::object>("y");
	sol::object z = lua.get<sol::object>("z");
	REQUIRE((y.as<int>() == 7));
	REQUIRE((z.as<int>() == 9));
}

TEST_CASE("usertype/usertype-utility", "Show internal management of classes registered through new_usertype") {
	sol::state lua;

	lua.new_usertype<fuser>("fuser", "add", &fuser::add, "add2", &fuser::add2);

	lua.script("a = fuser.new()\n"
		"b = a:add(1)\n"
		"c = a:add2(1)\n");

	sol::object a = lua.get<sol::object>("a");
	sol::object b = lua.get<sol::object>("b");
	sol::object c = lua.get<sol::object>("c");
	REQUIRE((a.is<sol::userdata_value>()));
	auto atype = a.get_type();
	auto btype = b.get_type();
	auto ctype = c.get_type();
	REQUIRE((atype == sol::type::userdata));
	REQUIRE((btype == sol::type::number));
	REQUIRE((ctype == sol::type::number));
	int bresult = b.as<int>();
	int cresult = c.as<int>();
	REQUIRE(bresult == 1);
	REQUIRE(cresult == 3);
}

TEST_CASE("usertype/usertype-utility-derived", "usertype classes must play nice when a derived class does not overload a publically visible base function") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);
	sol::constructors<sol::types<int>> basector;
	sol::usertype<Base> baseusertype(basector, "get_num", &Base::get_num);

	lua.set_usertype(baseusertype);

	lua.script("base = Base.new(5)");
	REQUIRE_NOTHROW(lua.script("print(base:get_num())"));

	sol::constructors<sol::types<int>> derivedctor;
	sol::usertype<Derived> derivedusertype(derivedctor,
		"get_num_10", &Derived::get_num_10,
		"get_num", &Derived::get_num
	);

	lua.set_usertype(derivedusertype);

	lua.script("derived = Derived.new(7)");
	lua.script("dgn = derived:get_num()\n"
		"print(dgn)");
	lua.script("dgn10 = derived:get_num_10()\n"
		"print(dgn10)");

	REQUIRE((lua.get<int>("dgn10") == 70));
	REQUIRE((lua.get<int>("dgn") == 7));
}

TEST_CASE("usertype/self-referential usertype", "usertype classes must play nice when C++ object types are requested for C++ code") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.new_usertype<self_test>("test", "g", &self_test::g, "f", &self_test::f);

	lua.script(
		"local a = test.new()\n"
		"a:g(\"woof\")\n"
		"a:f(a)\n"
	);
}

TEST_CASE("usertype/issue-number-twenty-five", "Using pointers and references from C++ classes in Lua") {
	struct test {
		int x = 0;
		test& set() {
			x = 10;
			return *this;
		}

		int get() {
			return x;
		}

		test* pget() {
			return this;
		}

		test create_get() {
			return *this;
		}

		int fun(int xa) {
			return xa * 10;
		}
	};

	sol::state lua;
	lua.open_libraries(sol::lib::base);
	lua.new_usertype<test>("test", "set", &test::set, "get", &test::get, "pointer_get", &test::pget, "fun", &test::fun, "create_get", &test::create_get);
	REQUIRE_NOTHROW(lua.script("x = test.new()"));
	REQUIRE_NOTHROW(lua.script("assert(x:set():get() == 10)"));
	REQUIRE_NOTHROW(lua.script("y = x:pointer_get()"));
	REQUIRE_NOTHROW(lua.script("y:set():get()"));
	REQUIRE_NOTHROW(lua.script("y:fun(10)"));
	REQUIRE_NOTHROW(lua.script("x:fun(10)"));
	REQUIRE_NOTHROW(lua.script("assert(y:fun(10) == x:fun(10), '...')"));
	REQUIRE_NOTHROW(lua.script("assert(y:fun(10) == 100, '...')"));
	REQUIRE_NOTHROW(lua.script("assert(y:set():get() == y:set():get(), '...')"));
	REQUIRE_NOTHROW(lua.script("assert(y:set():get() == 10, '...')"));
}

TEST_CASE("usertype/issue-number-thirty-five", "using value types created from lua-called C++ code, fixing user-defined types with constructors") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	sol::constructors<sol::types<float, float, float>> ctor;
	sol::usertype<Vec> udata(ctor, "normalized", &Vec::normalized, "length", &Vec::length);
	lua.set_usertype(udata);

	REQUIRE_NOTHROW(lua.script("v = Vec.new(1, 2, 3)\n"
		"print(v:length())"));
	REQUIRE_NOTHROW(lua.script("v = Vec.new(1, 2, 3)\n"
		"print(v:normalized():length())"));
}

TEST_CASE("usertype/lua-stored-usertype", "ensure usertype values can be stored without keeping usertype object alive") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	{
		sol::constructors<sol::types<float, float, float>> ctor;
		sol::usertype<Vec> udata(ctor,
			"normalized", &Vec::normalized,
			"length", &Vec::length);

		lua.set_usertype(udata);
		// usertype dies, but still usable in lua!
	}

	REQUIRE_NOTHROW(lua.script("collectgarbage()\n"
		"v = Vec.new(1, 2, 3)\n"
		"print(v:length())"));

	REQUIRE_NOTHROW(lua.script("v = Vec.new(1, 2, 3)\n"
		"print(v:normalized():length())"));
}

TEST_CASE("usertype/member-variables", "allow table-like accessors to behave as member variables for usertype") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);
	sol::constructors<sol::types<float, float, float>> ctor;
	sol::usertype<Vec> udata(ctor,
		"x", &Vec::x,
		"y", &Vec::y,
		"z", &Vec::z,
		"normalized", &Vec::normalized,
		"length", &Vec::length);
	lua.set_usertype(udata);

	REQUIRE_NOTHROW(lua.script("v = Vec.new(1, 2, 3)\n"
		"v2 = Vec.new(0, 1, 0)\n"
		"print(v:length())\n"
	));
	REQUIRE_NOTHROW(lua.script("v.x = 2\n"
		"v2.y = 2\n"
		"print(v.x, v.y, v.z)\n"
		"print(v2.x, v2.y, v2.z)\n"
	));
	REQUIRE_NOTHROW(lua.script("assert(v.x == 2)\n"
		"assert(v2.x == 0)\n"
		"assert(v2.y == 2)\n"
	));
	REQUIRE_NOTHROW(lua.script("v.x = 3\n"
		"local x = v.x\n"
		"assert(x == 3)\n"
	));

	struct breaks {
		sol::function f;
	};

	lua.open_libraries(sol::lib::base);
	lua.set("b", breaks());
	lua.new_usertype<breaks>("breaks",
		"f", &breaks::f
		);

	breaks& b = lua["b"];
	REQUIRE_NOTHROW(lua.script("b.f = function () print('BARK!') end"));
	REQUIRE_NOTHROW(lua.script("b.f()"));
	REQUIRE_NOTHROW(b.f());
}

TEST_CASE("usertype/nonmember-functions", "let users set non-member functions that take unqualified T as first parameter to usertype") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.new_usertype<giver>("giver",
		"gief_stuff", giver::gief_stuff,
		"gief", &giver::gief,
		"__tostring", [](const giver& t) {
		return std::to_string(t.a) + ": giving value";
	}
		).get<sol::table>("giver")
		.set_function("stuff", giver::stuff);

	REQUIRE_NOTHROW(lua.script("giver.stuff()"));
	REQUIRE_NOTHROW(lua.script("t = giver.new()\n"
		"print(tostring(t))\n"
		"t:gief()\n"
		"t:gief_stuff(20)\n"));
	giver& g = lua.get<giver>("t");
	REQUIRE(g.a == 20);
}

TEST_CASE("usertype/unique-shared-ptr", "manage the conversion and use of unique and shared pointers ('unique usertypes')") {
	const int64_t unique_value = 0x7125679355635963;
	auto uniqueint = std::make_unique<int64_t>(unique_value);
	auto sharedint = std::make_shared<int64_t>(unique_value);
	long preusecount = sharedint.use_count();
	{ sol::state lua;
	lua.open_libraries(sol::lib::base);
	lua.set("uniqueint", std::move(uniqueint));
	lua.set("sharedint", sharedint);
	std::unique_ptr<int64_t>& uniqueintref = lua["uniqueint"];
	std::shared_ptr<int64_t>& sharedintref = lua["sharedint"];
	int64_t* rawuniqueintref = lua["uniqueint"];
	int64_t* rawsharedintref = lua["sharedint"];
	int siusecount = sharedintref.use_count();
	REQUIRE((uniqueintref.get() == rawuniqueintref && sharedintref.get() == rawsharedintref));
	REQUIRE((uniqueintref != nullptr && sharedintref != nullptr && rawuniqueintref != nullptr && rawsharedintref != nullptr));
	REQUIRE((unique_value == *uniqueintref.get() && unique_value == *sharedintref.get()));
	REQUIRE((unique_value == *rawuniqueintref && unique_value == *rawsharedintref));
	REQUIRE(siusecount == sharedint.use_count());
	std::shared_ptr<int64_t> moreref = sharedint;
	REQUIRE(unique_value == *moreref.get());
	REQUIRE(moreref.use_count() == sharedint.use_count());
	REQUIRE(moreref.use_count() == sharedintref.use_count());
	}
	REQUIRE(preusecount == sharedint.use_count());
}

TEST_CASE("regressions/one", "issue number 48") {
	sol::state lua;
	lua.new_usertype<vars>("vars",
		"boop", &vars::boop);
	REQUIRE_NOTHROW(lua.script("beep = vars.new()\n"
		"beep.boop = 1"));
	// test for segfault
	auto my_var = lua.get<vars>("beep");
	REQUIRE(my_var.boop == 1);
	auto* ptr = &my_var;
	REQUIRE(ptr->boop == 1);
}

TEST_CASE("usertype/get-set-references", "properly get and set with std::ref semantics. Note that to get, we must not use Unqualified<T> on the type...") {
	sol::state lua;

	lua.new_usertype<vars>("vars",
		"boop", &vars::boop);
	vars var{};
	vars rvar{};
	lua.set("beep", var);
	lua.set("rbeep", std::ref(rvar));
	auto& my_var = lua.get<vars>("beep");
	auto& ref_var = lua.get<std::reference_wrapper<vars>>("rbeep");
	vars& proxy_my_var = lua["beep"];
	std::reference_wrapper<vars> proxy_ref_var = lua["rbeep"];
	var.boop = 2;
	rvar.boop = 5;

	// Was return as a value: var must be diferent from "beep"
	REQUIRE_FALSE(std::addressof(var) == std::addressof(my_var));
	REQUIRE_FALSE(std::addressof(proxy_my_var) == std::addressof(var));
	REQUIRE((my_var.boop == 0));
	REQUIRE(var.boop != my_var.boop);

	REQUIRE(std::addressof(ref_var) == std::addressof(rvar));
	REQUIRE(std::addressof(proxy_ref_var.get()) == std::addressof(rvar));
	REQUIRE(rvar.boop == 5);
	REQUIRE(rvar.boop == ref_var.boop);
}

TEST_CASE("usertype/destructor-tests", "Show that proper copies / destruction happens") {
	static int created = 0;
	static int destroyed = 0;
	static void* last_call = nullptr;
	struct x {
		x() { ++created; }
		x(const x&) { ++created; }
		x(x&&) { ++created; }
		x& operator=(const x&) { return *this; }
		x& operator=(x&&) { return *this; }
		~x() { ++destroyed; }
	};
	{
		sol::state lua;
		lua.new_usertype<x>("x");
		x x1;
		x x2;
		lua.set("x1copy", x1, "x2copy", x2, "x1ref", std::ref(x1));
		x& x1copyref = lua["x1copy"];
		x& x2copyref = lua["x2copy"];
		x& x1ref = lua["x1ref"];
		REQUIRE(created == 4);
		REQUIRE(destroyed == 0);
		REQUIRE(std::addressof(x1) == std::addressof(x1ref));
		REQUIRE(std::addressof(x1copyref) != std::addressof(x1));
		REQUIRE(std::addressof(x2copyref) != std::addressof(x2));
	}
	REQUIRE(created == 4);
	REQUIRE(destroyed == 4);
}

TEST_CASE("usertype/private-constructible", "Check to make sure special snowflake types from Enterprise thingamahjongs work properly.") {
	int numsaved = factory_test::num_saved;
	int numkilled = factory_test::num_killed;
	{
		sol::state lua;
		lua.open_libraries(sol::lib::base);

		lua.new_usertype<factory_test>("factory_test",
			"new", sol::initializers(factory_test::save),
			"__gc", sol::destructor(factory_test::kill),
			"a", &factory_test::a
			);

		std::unique_ptr<factory_test, factory_test::deleter> f = factory_test::make();
		lua.set("true_a", factory_test::true_a, "f", f.get());
		REQUIRE_NOTHROW(lua.script("assert(f.a == true_a)"));

		REQUIRE_NOTHROW(lua.script(
			"local fresh_f = factory_test:new()\n"
			"assert(fresh_f.a == true_a)\n"));
	}
	int expectednumsaved = numsaved + 1;
	int expectednumkilled = numkilled + 1;
	REQUIRE(expectednumsaved == factory_test::num_saved);
	REQUIRE(expectednumkilled == factory_test::num_killed);
}

TEST_CASE("usertype/const-pointer", "Make sure const pointers can be taken") {
	struct A { int x = 201; };
	struct B {
		int foo(const A* a) { return a->x; };
	};

	sol::state lua;
	lua.new_usertype<B>("B", 
		"foo", &B::foo
	);
	lua.set("a", A());
	lua.set("b", B());
	lua.script("x = b:foo(a)");
	int x = lua["x"];
	REQUIRE(x == 201);
}

TEST_CASE("usertype/overloading", "Check if overloading works properly for usertypes") {
	struct woof {
		int var;

		int func(int x) {
			return var + x;
		}

		double func2(int x, int y) {
			return var + x + y + 0.5;
		}

		std::string func2s(int x, std::string y) {
			return y + " " + std::to_string(x);
		}
	};
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.new_usertype<woof>("woof",
		"var", &woof::var,
		"func", sol::overload(&woof::func, &woof::func2, &woof::func2s)
		);

	const std::string bark_58 = "bark 58";

	REQUIRE_NOTHROW(lua.script(
		"r = woof:new()\n"
		"a = r:func(1)\n"
		"b = r:func(1, 2)\n"
		"c = r:func(58, 'bark')\n"
	));
	REQUIRE((lua["a"] == 1));
	REQUIRE((lua["b"] == 3.5));
	REQUIRE((lua["c"] == bark_58));

	REQUIRE_THROWS(lua.script("r:func(1,2,'meow')"));
}

TEST_CASE("usertype/overloading_values", "ensure overloads handle properly") {
	struct overloading_test {
		int print(int i) { INFO("Integer print: " << i); return 500 + i; }
		int print() { INFO("No param print."); return 500; }
	};

	sol::state lua;
	lua.new_usertype<overloading_test>("overloading_test", sol::constructors<>(),
		"print", sol::overload(static_cast<int (overloading_test::*)(int)>(&overloading_test::print), static_cast<int (overloading_test::*)()>(&overloading_test::print)),
		"print2", sol::overload(static_cast<int (overloading_test::*)()>(&overloading_test::print), static_cast<int (overloading_test::*)(int)>(&overloading_test::print))
		);
	lua.set("test", overloading_test());

	sol::function f0_0 = lua.load("return test:print()");
	sol::function f0_1 = lua.load("return test:print2()");
	sol::function f1_0 = lua.load("return test:print(24)");
	sol::function f1_1 = lua.load("return test:print2(24)");
	int res = f0_0();
	int res2 = f0_1();
	int res3 = f1_0();
	int res4 = f1_1();

	REQUIRE(res == 500);
	REQUIRE(res2 == 500);

	REQUIRE(res3 == 524);
	REQUIRE(res4 == 524);
}

TEST_CASE("usertype/reference-and-constness", "Make sure constness compiles properly and errors out at runtime") {
	struct bark {
		int var = 50;
	};
	struct woof {
		bark b;
	};

	struct nested {
		const int f = 25;
	};

	struct outer {
		nested n;
	};

	sol::state lua;
	lua.new_usertype<woof>("woof",
		"b", &woof::b);
	lua.new_usertype<bark>("bark",
		"var", &bark::var);
	lua.new_usertype<outer>("outer",
		"n", &outer::n);
	lua.set("w", woof());
	lua.set("n", nested());
	lua.set("o", outer());
	lua.set("f", sol::c_call<decltype(&nested::f), &nested::f>);
	lua.script(R"(
    x = w.b
    x.var = 20
    val = w.b.var == x.var
    v = f(n);
    )");

	woof& w = lua["w"];
	bark& x = lua["x"];
	nested& n = lua["n"];
	int v = lua["v"];
	bool val = lua["val"];
	// enforce reference semantics
	REQUIRE(std::addressof(w.b) == std::addressof(x));
	REQUIRE(n.f == 25);
	REQUIRE(v == 25);
	REQUIRE(val);

	REQUIRE_THROWS(lua.script("f(n, 50)"));
	REQUIRE_THROWS(lua.script("o.n = 25"));
}

TEST_CASE("usertype/readonly-and-static-functions", "Check if static functions can be called on userdata and from their originating (meta)tables") {
	struct bark {
		int var = 50;

		void func() {}

		static void oh_boy() {}

		static int oh_boy(std::string name) {
			return static_cast<int>(name.length());
		}

		int operator()(int x) {
			return x;
		}
	};

	sol::state lua;
	lua.open_libraries(sol::lib::base);
	lua.new_usertype<bark>("bark",
		"var", &bark::var,
		"var2", sol::readonly(&bark::var),
		"something", something,
		"something2", [](int x, int y) { return x + y; },
		"func", &bark::func,
		"oh_boy", sol::overload(sol::resolve<void()>(&bark::oh_boy), sol::resolve<int(std::string)>(&bark::oh_boy)),
		sol::meta_function::call_function, &bark::operator()
		);

	REQUIRE_NOTHROW(lua.script("assert(bark.oh_boy('woo') == 3)"));
	REQUIRE_NOTHROW(lua.script("bark.oh_boy()"));

	bark b;
	lua.set("b", &b);

	sol::table b_table = lua["b"];
	sol::function member_func = b_table["func"];
	sol::function s = b_table["something"];
	sol::function s2 = b_table["something2"];

	sol::table b_metatable = b_table[sol::metatable_key];
	bool isvalidmt = b_metatable.valid();
	REQUIRE(isvalidmt);
	sol::function b_call = b_metatable["__call"];
	sol::function b_as_function = lua["b"];

	int x = b_as_function(1);
	int y = b_call(b, 1);
	bool z = s();
	int w = s2(2, 3);
	REQUIRE(x == 1);
	REQUIRE(y == 1);
	REQUIRE(z);
	REQUIRE(w == 5);

	lua.script(R"(
lx = b(1)
ly = getmetatable(b).__call(b, 1)
lz = b.something()
lz2 = bark.something()
lw = b.something2(2, 3)
lw2 = bark.something2(2, 3)
    )");

	int lx = lua["lx"];
	int ly = lua["ly"];
	bool lz = lua["lz"];
	int lw = lua["lw"];
	bool lz2 = lua["lz2"];
	int lw2 = lua["lw2"];
	REQUIRE(lx == 1);
	REQUIRE(ly == 1);
	REQUIRE(lz);
	REQUIRE(lz2);
	REQUIRE(lw == 5);
	REQUIRE(lw2 == 5);
	REQUIRE(lx == ly);
	REQUIRE(lz == lz2);
	REQUIRE(lw == lw2);

	REQUIRE_THROWS(lua.script("b.var2 = 2"));
}

TEST_CASE("usertype/properties", "Check if member properties/variables work") {
	struct bark {
		int var = 50;
		int var2 = 25;

		int get_var2() const {
			return var2;
		}

		int get_var3() {
			return var2;
		}

		void set_var2(int x) {
			var2 = x;
		}
	};

	sol::state lua;
	lua.open_libraries(sol::lib::base);
	lua.new_usertype<bark>("bark",
		"var", &bark::var,
		"var2", sol::readonly(&bark::var2),
		"a", sol::property(&bark::get_var2, &bark::set_var2),
		"b", sol::property(&bark::get_var2),
		"c", sol::property(&bark::get_var3),
		"d", sol::property(&bark::set_var2)
		);

	bark b;
	lua.set("b", &b);

	lua.script("b.a = 59");
	lua.script("var2_0 = b.a");
	lua.script("var2_1 = b.b");
	lua.script("b.d = 1568");
	lua.script("var2_2 = b.c");

	int var2_0 = lua["var2_0"];
	int var2_1 = lua["var2_1"];
	int var2_2 = lua["var2_2"];
	REQUIRE(var2_0 == 59);
	REQUIRE(var2_1 == 59);
	REQUIRE(var2_2 == 1568);

	REQUIRE_THROWS(lua.script("b.var2 = 24"));
	REQUIRE_THROWS(lua.script("r = b.d"));
	REQUIRE_THROWS(lua.script("r = b.d"));
	REQUIRE_THROWS(lua.script("b.b = 25"));
	REQUIRE_THROWS(lua.script("b.c = 11"));
}

TEST_CASE("usertype/safety", "crash with an exception -- not a segfault -- on bad userdata calls") {
	class Test {
	public:
		void sayHello() { std::cout << "Hey\n"; }
	};

	sol::state lua;
	lua.new_usertype<Test>("Test", "sayHello", &Test::sayHello);
	static const std::string code = R"(
        local t = Test.new()
        t:sayHello() --Works fine
        t.sayHello() --Uh oh.
    )";
	REQUIRE_THROWS(lua.script(code));
}

TEST_CASE("usertype/call_constructor", "make sure lua types can be constructed with function call constructors") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.new_usertype<thing>("thing",
		"v", &thing::v
		, sol::call_constructor, sol::constructors<sol::types<>, sol::types<int>>()
		);

	lua.script(R"(
t = thing(256)
)");

	thing& y = lua["t"];
	INFO(y.v);
	REQUIRE(y.v == 256);
}

TEST_CASE("usertype/call_constructor_2", "prevent metatable regression") {
	class class01 {
	public:
		int x = 57;
		class01() {}
	};

	class class02 {
	public:
		int x = 50;
		class02() {}
		class02(const class01& other) : x(other.x) {}
	};

	sol::state lua;

	lua.new_usertype<class01>("class01",
		sol::call_constructor, sol::constructors<sol::types<>, sol::types<const class01&>>()
	);

	lua.new_usertype<class02>("class02",
		sol::call_constructor, sol::constructors<sol::types<>, sol::types<const class02&>, sol::types<const class01&>>()
	);

	REQUIRE_NOTHROW(lua.script(R"(
x = class01()
y = class02(x)
)"));
	class02& y = lua["y"];
	REQUIRE(y.x == 57);
}

TEST_CASE("usertype/blank_constructor", "make sure lua types cannot be constructed if a blank / empty constructor is provided") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.new_usertype<thing>("thing",
		"v", &thing::v
		, sol::call_constructor, sol::constructors<>()
		);

	REQUIRE_THROWS(lua.script("t = thing(256)"));
}


TEST_CASE("usertype/no_constructor", "make sure lua types cannot be constructed if a blank / empty constructor is provided") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	SECTION("order1")
	{
		lua.new_usertype<thing>("thing",
		"v", &thing::v
		, sol::call_constructor, sol::no_constructor
		);
		REQUIRE_THROWS(lua.script("t = thing.new()"));
	}

	SECTION("order2")
	{
		lua.new_usertype<thing>("thing"
			, sol::call_constructor, sol::no_constructor
			, "v", &thing::v
		);
		REQUIRE_THROWS(lua.script("t = thing.new()"));
	}
	
	REQUIRE_THROWS(lua.script("t = thing.new()"));
}

TEST_CASE("usertype/coverage", "try all the things") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.new_usertype<ext_getset>("ext_getset",
		sol::call_constructor, sol::constructors<sol::types<>, sol::types<int>>(),
		sol::meta_function::garbage_collect, sol::destructor(des<ext_getset>),
		"x", sol::overload(&ext_getset::x, &ext_getset::x2, [](ext_getset& m, std::string x, int y) { 
			return m.meow + 50 + y + x.length(); 
		}),
		"bark", &ext_getset::bark,
		"meow", &ext_getset::meow,
		"readonlybark", sol::readonly(&ext_getset::bark),
		"set", &ext_getset::set,
		"get", &ext_getset::get,
		"sset", &ext_getset::s_set,
		"sget", &ext_getset::s_get,
		"propbark", sol::property(&ext_getset::set, &ext_getset::get),
		"readonlypropbark", sol::property(&ext_getset::get),
		"writeonlypropbark", sol::property(&ext_getset::set)
		);

	INFO("usertype created");

	lua.script(R"(
e = ext_getset()
w = e:x(e:x(), e:x(e:x()))
print(w)
)");

	int w = lua["w"];
	REQUIRE(w == (56 + 50 + 14 + 14));

	INFO("REQUIRE(w) successful");

	lua.script(R"(
e:set(500)
e.sset(24)
x = e:get()
y = e.sget(20)
)");

	int x = lua["x"];
	int y = lua["y"];
	REQUIRE(x == 500);
	REQUIRE(y == 40);
	
	INFO("REQUIRE(x, y) successful");

	lua.script(R"(
e.bark = 5001
a = e:get()
print(e.bark)
print(a)

e.propbark = 9700
b = e:get()
print(e.propbark)
print(b)
)");
	int a = lua["a"];
	int b = lua["b"];

	REQUIRE(a == 5001);
	REQUIRE(b == 9700);

	INFO("REQUIRE(a, b) successful");

	lua.script(R"(
c = e.readonlybark
d = e.meow
print(e.readonlybark)
print(c)
print(e.meow)
print(d)
)");

	int c = lua["c"];
	int d = lua["d"];
	REQUIRE(c == 9700);
	REQUIRE(d == 56);

	INFO("REQUIRE(c, d) successful");

	lua.script(R"(
e.writeonlypropbark = 500
z = e.readonlypropbark
print(e.readonlybark)
print(e.bark)
)");

	int z = lua["z"];
	REQUIRE(z == 500);

	INFO("REQUIRE(z) successful");

	REQUIRE_THROWS(lua.script("e.readonlybark = 24"));
	INFO("REQUIRE_THROWS 1 successful");
	REQUIRE_THROWS(lua.script("e.readonlypropbark = 500"));
	INFO("REQUIRE_THROWS 2 successful");
	REQUIRE_THROWS(lua.script("y = e.writeonlypropbark"));
	INFO("REQUIRE_THROWS 3 successful");

}

TEST_CASE("usertype/copyability", "make sure user can write to a class variable even if the class itself isn't copy-safe") {
	struct NoCopy {
		int get() const { return _you_can_copy_me; }
		void set(int val) { _you_can_copy_me = val; }

		int _you_can_copy_me;
		std::mutex _haha_you_cant_copy_me;
	};

	sol::state lua;
	lua.new_usertype<NoCopy>("NoCopy", "val", sol::property(&NoCopy::get, &NoCopy::set));

	REQUIRE_NOTHROW(
		lua.script(R"__(
nocopy = NoCopy.new()
nocopy.val = 5
               )__")
	);
}

TEST_CASE("usertype/protect", "users should be allowed to manually protect a function") {
	struct protect_me {
		int gen(int x) {
			return x;
		}
	};

	sol::state lua;
	lua.open_libraries(sol::lib::base);
	lua.new_usertype<protect_me>("protect_me", 
		"gen", sol::protect( &protect_me::gen )
	);

	REQUIRE_NOTHROW(
		lua.script(R"__(
pm = protect_me.new()
value = pcall(pm.gen,pm)
)__");
	);
	bool value = lua["value"];
	REQUIRE_FALSE(value);
}

TEST_CASE("usertype/shared-ptr-regression", "usertype metatables should not screw over unique usertype metatables") {
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

		lua.new_usertype<test>("test",
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

TEST_CASE("usertype/double-deleter-guards", "usertype metatables internally must not rely on internal ") {
	struct c_a { int x; };
	struct c_b { int y; };
	REQUIRE_NOTHROW( {
		sol::state lua;
		lua.new_usertype<c_a>("c_a", "x", &c_a::x);
		lua.new_usertype<c_b>("c_b", "y", &c_b::y);
		lua = sol::state();
		lua.new_usertype<c_a>("c_a", "x", &c_a::x);
		lua.new_usertype<c_b>("c_b", "y", &c_b::y);
		lua = sol::state();
	});
}

TEST_CASE("usertype/vars", "usertype vars can bind various class items") {
	static int muh_variable = 25;
	static int through_variable = 10;

	sol::state lua;
	lua.open_libraries();
	struct test {};
	lua.new_usertype<test>("test",
		"straight", sol::var(2),
		"global", sol::var(muh_variable),
		"ref_global", sol::var(std::ref(muh_variable)),
		"global2", sol::var(through_variable),
		"ref_global2", sol::var(std::ref(through_variable))
		);

	int prets = lua["test"]["straight"];
	int pretg = lua["test"]["global"];
	int pretrg = lua["test"]["ref_global"];
	int pretg2 = lua["test"]["global2"];
	int pretrg2 = lua["test"]["ref_global2"];

	REQUIRE(prets == 2);
	REQUIRE(pretg == 25);
	REQUIRE(pretrg == 25);
	REQUIRE(pretg2 == 10);
	REQUIRE(pretrg2 == 10);

	lua.script(R"(
print(test.straight)
test.straight = 50
print(test.straight)
)");
	int s = lua["test"]["straight"];
	REQUIRE(s == 50);

	lua.script(R"(
t = test.new()
print(t.global)
t.global = 50
print(t.global)
)");
	int mv = lua["test"]["global"];
	REQUIRE(mv == 50);
	REQUIRE(muh_variable == 25);


	lua.script(R"(
print(t.ref_global)
t.ref_global = 50
print(t.ref_global)
)");
	int rmv = lua["test"]["ref_global"];
	REQUIRE(rmv == 50);
	REQUIRE(muh_variable == 50);

	REQUIRE(through_variable == 10);
	lua.script(R"(
print(test.global2)
test.global2 = 35
print(test.global2)
)");
	int tv = lua["test"]["global2"];
	REQUIRE(through_variable == 10);
	REQUIRE(tv == 35);

	lua.script(R"(
print(test.ref_global2)
test.ref_global2 = 35
print(test.ref_global2)
)");
	int rtv = lua["test"]["ref_global2"];
	REQUIRE(rtv == 35);
	REQUIRE(through_variable == 35);
}

TEST_CASE("usertypes/var-and-property", "make sure const vars are readonly and properties can handle lambdas") {
	const static int arf = 20;

	struct test {
		int value = 10;
	};

	sol::state lua;
	lua.open_libraries();

	lua.new_usertype<test>("test",
		"prop", sol::property(
			[](test& t) {
				return t.value;
			},
			[](test& t, int x) {
				t.value = x;
			}
		),
		"global", sol::var(std::ref(arf))
		);

	lua.script(R"(
t = test.new()
print(t.prop)
t.prop = 50
print(t.prop)
	)");

	test& t = lua["t"];
	REQUIRE(t.value == 50);


	REQUIRE_THROWS(
		lua.script(R"(
t = test.new()
print(t.global)
t.global = 20
print(t.global)
	)"));
}

TEST_CASE("usertype/unique_usertype-check", "make sure unique usertypes don't get pushed as references with function calls and the like") {
	class Entity {
	public:
		std::string GetName() {
			return "Charmander";
		}
	};

	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::math, sol::lib::string, sol::lib::io);

	lua.new_usertype<Entity>("Entity",
		"new", sol::no_constructor,
		"get_name", &Entity::GetName
		);

	lua.script(R"(
		function my_func(entity)
		print("INSIDE LUA")
		print(entity:get_name())
		end
)");

	sol::function my_func = lua["my_func"];
	REQUIRE_NOTHROW({
	auto ent = std::make_shared<Entity>();
	my_func(ent);
	Entity ent2;
	my_func(ent2);
	my_func(std::make_shared<Entity>());
	});
}

TEST_CASE("usertype/abstract-base-class", "Ensure that abstract base classes and such can be registered") {	
	sol::state lua;
	lua.new_usertype<abstract_A>("A", "a", &abstract_A::a);
	lua.new_usertype<abstract_B>("B", sol::base_classes, sol::bases<abstract_A>());
	lua.script(R"(local b = B.new()
b:a()
)");
}

TEST_CASE("usertype/as_function", "Ensure that variables can be turned into functions by as_function") {
	class B {
	public:
		int bvar = 24;
	};

	sol::state lua;
	lua.open_libraries();
	lua.new_usertype<B>("B", "b", &B::bvar, "f", sol::as_function(&B::bvar));

	B b;
	lua.set("b", &b);
	lua.script("x = b:f()");
	lua.script("y = b.b");
	int x = lua["x"];
	int y = lua["y"];
	REQUIRE(x == 24);
	REQUIRE(y == 24);
}

TEST_CASE("usertype/destruction-test", "make sure usertypes are properly destructed and don't double-delete memory or segfault") {
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

	lua.new_usertype<CrashClass>("CrashClass",
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
