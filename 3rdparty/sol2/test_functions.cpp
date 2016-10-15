#define SOL_CHECK_ARGUMENTS

#include <catch.hpp>
#include <sol.hpp>
#include <iostream>
#include "test_stack_guard.hpp"

std::function<int()> makefn() {
	auto fx = []() -> int {
		return 0x1456789;
	};
	return fx;
}

void takefn(std::function<int()> purr) {
	if (purr() != 0x1456789)
		throw 0;
}

struct A {
	int a = 0xA; int bark() { return 1; }
};

std::tuple<int, int> bark(int num_value, A* a) {
	return std::tuple<int, int>(num_value * 2, a->bark());
}

void test_free_func(std::function<void()> f) {
	f();
}

void test_free_func2(std::function<int(int)> f, int arg1) {
	int val = f(arg1);
	if (val != arg1)
		throw sol::error("failed function call!");
}

int overloaded(int x) {
	INFO(x);
	return 3;
}

int overloaded(int x, int y) {
	INFO(x << " " << y);
	return 7;
}

int overloaded(int x, int y, int z) {
	INFO(x << " " << y << " " << z);
	return 11;
}

int non_overloaded(int x, int y, int z) {
	INFO(x << " " << y << " " << z);
	return 13;
}

namespace sep {
	int plop_xyz(int x, int y, std::string z) {
		INFO(x << " " << y << " " << z);
		return 11;
	}
}

int func_1(int) {
	return 1;
}

std::string func_1s(std::string a) {
	return "string: " + a;
}

int func_2(int, int) {
	return 2;
}

void func_3(int, int, int) {

}

int f1(int) { return 32; }
int f2(int, int) { return 1; }
struct fer {
	double f3(int, int) {
		return 2.5;
	}
};

TEST_CASE("functions/tuple-returns", "Make sure tuple returns are ordered properly") {
	sol::state lua;
	lua.script("function f() return '3', 4 end");

	std::tuple<std::string, int> result = lua["f"]();
	auto s = std::get<0>(result);
	auto v = std::get<1>(result);
	REQUIRE(s == "3");
	REQUIRE(v == 4);
}

TEST_CASE("functions/overload-resolution", "Check if overloaded function resolution templates compile/work") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.set_function("non_overloaded", non_overloaded);
	REQUIRE_NOTHROW(lua.script("x = non_overloaded(1, 2, 3)\nprint(x)"));

	/*
	// Cannot reasonably support: clang++ refuses to try enough
	// deductions to make this work
	lua.set_function<int>("overloaded", overloaded);
	REQUIRE_NOTHROW(lua.script("print(overloaded(1))"));

	lua.set_function<int, int>("overloaded", overloaded);
	REQUIRE_NOTHROW(lua.script("print(overloaded(1, 2))"));

	lua.set_function<int, int, int>("overloaded", overloaded);
	REQUIRE_NOTHROW(lua.script("print(overloaded(1, 2, 3))"));
	*/
	lua.set_function("overloaded", sol::resolve<int(int)>(overloaded));
	REQUIRE_NOTHROW(lua.script("print(overloaded(1))"));

	lua.set_function("overloaded", sol::resolve<int(int, int)>(overloaded));
	REQUIRE_NOTHROW(lua.script("print(overloaded(1, 2))"));

	lua.set_function("overloaded", sol::resolve<int(int, int, int)>(overloaded));
	REQUIRE_NOTHROW(lua.script("print(overloaded(1, 2, 3))"));
}

TEST_CASE("functions/return-order-and-multi-get", "Check if return order is in the same reading order specified in Lua") {
	const static std::tuple<int, int, int> triple = std::make_tuple(10, 11, 12);
	const static std::tuple<int, float> paired = std::make_tuple(10, 10.f);
	sol::state lua;
	lua.set_function("f", [] {
		return std::make_tuple(10, 11, 12);
	});
	int a = 0;
	lua.set_function("h", []() {
		return std::make_tuple(10, 10.0f);
	});
	lua.script("function g() return 10, 11, 12 end\nx,y,z = g()");
	auto tcpp = lua.get<sol::function>("f").call<int, int, int>();
	auto tlua = lua.get<sol::function>("g").call<int, int, int>();
	auto tcpp2 = lua.get<sol::function>("h").call<int, float>();
	auto tluaget = lua.get<int, int, int>("x", "y", "z");
	REQUIRE(tcpp == triple);
	REQUIRE(tlua == triple);
	REQUIRE(tluaget == triple);
	REQUIRE(tcpp2 == paired);
}

TEST_CASE("functions/deducing-return-order-and-multi-get", "Check if return order is in the same reading order specified in Lua, with regular deducing calls") {
	const static std::tuple<int, int, int> triple = std::make_tuple(10, 11, 12);
	sol::state lua;
	lua.set_function("f_string", []() { return "this is a string!"; });
	sol::function f_string = lua["f_string"];

	// Make sure there are no overload collisions / compiler errors for automatic string conversions
	std::string f_string_result = f_string();
	REQUIRE(f_string_result == "this is a string!");
	f_string_result = f_string();
	REQUIRE(f_string_result == "this is a string!");

	lua.set_function("f", [] {
		return std::make_tuple(10, 11, 12);
	});
	lua.script("function g() return 10, 11, 12 end\nx,y,z = g()");
	std::tuple<int, int, int> tcpp = lua.get<sol::function>("f")();
	std::tuple<int, int, int> tlua = lua.get<sol::function>("g")();
	std::tuple<int, int, int> tluaget = lua.get<int, int, int>("x", "y", "z");
	INFO("cpp: " << std::get<0>(tcpp) << ',' << std::get<1>(tcpp) << ',' << std::get<2>(tcpp));
	INFO("lua: " << std::get<0>(tlua) << ',' << std::get<1>(tlua) << ',' << std::get<2>(tlua));
	INFO("lua xyz: " << lua.get<int>("x") << ',' << lua.get<int>("y") << ',' << lua.get<int>("z"));
	REQUIRE(tcpp == triple);
	REQUIRE(tlua == triple);
	REQUIRE(tluaget == triple);
}

TEST_CASE("functions/optional-values", "check if optionals can be passed in to be nil or otherwise") {
	struct thing {
		int v;
	};
	sol::state lua;
	lua.script(R"( function f (a)
    return a
end )");

	sol::function lua_bark = lua["f"];

	sol::optional<int> testv = lua_bark(sol::optional<int>(29));
	sol::optional<int> testn = lua_bark(sol::nullopt);
	REQUIRE((bool)testv);
	REQUIRE_FALSE((bool)testn);
	REQUIRE(testv.value() == 29);
	sol::optional<thing> v = lua_bark(sol::optional<thing>(thing{ 29 }));
	REQUIRE_NOTHROW(sol::nil_t n = lua_bark(sol::nullopt));
	REQUIRE(v->v == 29);
}

TEST_CASE("functions/pair-and-tuple-and-proxy-tests", "Check if sol::reference and sol::proxy can be passed to functions as arguments") {
	sol::state lua;
	lua.new_usertype<A>("A",
		"bark", &A::bark);
	lua.script(R"( function f (num_value, a)
    return num_value * 2, a:bark()
end 
function h (num_value, a, b)
    return num_value * 2, a:bark(), b * 3
end 
nested = { variables = { no = { problem = 10 } } } )");
	lua.set_function("g", bark);

	sol::function cpp_bark = lua["g"];
	sol::function lua_bark = lua["f"];
	sol::function lua_bark2 = lua["h"];

	sol::reference lua_variable_x = lua["nested"]["variables"]["no"]["problem"];
	A cpp_variable_y;

	static const std::tuple<int, int> abdesired(20, 1);
	static const std::pair<int, int> cddesired = { 20, 1 };
	static const std::tuple<int, int, int> abcdesired(20, 1, 3);

	std::tuple<int, int> ab = cpp_bark(lua_variable_x, cpp_variable_y);
	std::pair<int, int> cd = lua_bark(lua["nested"]["variables"]["no"]["problem"], cpp_variable_y);

	REQUIRE(ab == abdesired);
	REQUIRE(cd == cddesired);

	ab = cpp_bark(std::make_pair(lua_variable_x, cpp_variable_y));
	cd = lua_bark(std::make_pair(lua["nested"]["variables"]["no"]["problem"], cpp_variable_y));

	REQUIRE(ab == abdesired);
	REQUIRE(cd == cddesired);

	std::tuple<int, int, int> abc = lua_bark2(std::make_tuple(10, cpp_variable_y), sol::optional<int>(1));
	REQUIRE(abc == abcdesired);
}

TEST_CASE("functions/sol::function-to-std::function", "check if conversion to std::function works properly and calls with correct arguments") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.set_function("testFunc", test_free_func);
	lua.set_function("testFunc2", test_free_func2);
	lua.script(
		"testFunc(function() print(\"hello std::function\") end)"
	);
	REQUIRE_NOTHROW(lua.script(
		"function m(a)\n"
		"     print(\"hello std::function with arg \", a)\n"
		"     return a\n"
		"end\n"
		"\n"
		"testFunc2(m, 1)"
	));
}

TEST_CASE("functions/returning-functions-from-C++-and-gettin-in-lua", "check to see if returning a functor and getting a functor from lua is possible") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.set_function("makefn", makefn);
	lua.set_function("takefn", takefn);
	lua.script("afx = makefn()\n"
		"print(afx())\n"
		"takefn(afx)\n");
}

TEST_CASE("functions/function_result-protected_function_result", "Function result should be the beefy return type for sol::function that allows for error checking and error handlers") {
	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::debug);
	static const char unhandlederrormessage[] = "true error message";
	static const char handlederrormessage[] = "doodle";
	static const std::string handlederrormessage_s = handlederrormessage;

	// Some function; just using a lambda to be cheap
	auto doomfx = []() {
		INFO("doomfx called");
		throw std::runtime_error(unhandlederrormessage);
	};
	auto luadoomfx = [&lua]() {
		INFO("luadoomfx called");
		// Does not bypass error function, will call it
		luaL_error(lua.lua_state(), unhandlederrormessage);
	};
	lua.set_function("doom", doomfx);
	lua.set_function("luadoom", luadoomfx);

	auto cpphandlerfx = [](std::string x) {
		INFO("c++ handler called with: " << x);
		return handlederrormessage;
	};
	lua.set_function("cpphandler", cpphandlerfx);
	lua.script(
		std::string("function luahandler ( message )")
		+ "    print('lua handler called with: ' .. message)"
		+ "    return '" + handlederrormessage + "'"
		+ "end"
	);
	auto nontrampolinefx = [](lua_State*) -> int { throw "x"; };
	lua_CFunction c_nontrampolinefx = nontrampolinefx;
	lua.set("nontrampoline", c_nontrampolinefx);
	lua.set_function("bark", []() -> int {return 100; });

	sol::function luahandler = lua["luahandler"];
	sol::function cpphandler = lua["cpphandler"];
	sol::protected_function doom(lua["doom"], luahandler);
	sol::protected_function luadoom(lua["luadoom"]);
	sol::protected_function nontrampoline = lua["nontrampoline"];
	sol::protected_function justfine = lua["bark"];
	sol::protected_function justfinewithhandler = lua["bark"];
	luadoom.error_handler = cpphandler;
	nontrampoline.error_handler = cpphandler;
	justfinewithhandler.error_handler = luahandler;
	bool present = true;
	{
		sol::protected_function_result result = doom();
		REQUIRE_FALSE(result.valid());
		sol::optional<sol::error> operr = result;
		sol::optional<int> opvalue = result;
		present = (bool)operr;
		REQUIRE(present);
		present = (bool)opvalue;
		REQUIRE_FALSE(present);
		sol::error err = result;
		REQUIRE(err.what() == handlederrormessage_s);
	}
	{
		sol::protected_function_result result = luadoom();
		REQUIRE_FALSE(result.valid());
		sol::optional<sol::error> operr = result;
		sol::optional<int> opvalue = result;
		present = (bool)operr;
		REQUIRE(present);
		present = (bool)opvalue;
		REQUIRE_FALSE(present);
		sol::error err = result;
		REQUIRE(err.what() == handlederrormessage_s);
	}
	{
		sol::protected_function_result result = nontrampoline();
		REQUIRE_FALSE(result.valid());
		sol::optional<sol::error> operr = result;
		sol::optional<int> opvalue = result;
		present = (bool)operr;
		REQUIRE(present);
		present = (bool)opvalue;
		REQUIRE_FALSE(present);
		sol::error err = result;
		REQUIRE(err.what() == handlederrormessage_s);
	}
	{
		sol::protected_function_result result = justfine();
		REQUIRE(result.valid());
		sol::optional<sol::error> operr = result;
		sol::optional<int> opvalue = result;
		present = (bool)operr;
		REQUIRE_FALSE(present);
		present = (bool)opvalue;
		REQUIRE(present);
		int value = result;
		REQUIRE(value == 100);
	}
	{
		sol::protected_function_result result = justfinewithhandler();
		REQUIRE(result.valid());
		sol::optional<sol::error> operr = result;
		sol::optional<int> opvalue = result;
		present = (bool)operr;
		REQUIRE_FALSE(present);
		present = (bool)opvalue;
		REQUIRE(present);
		int value = result;
		REQUIRE(value == 100);
	}
}

TEST_CASE("functions/destructor-tests", "Show that proper copies / destruction happens") {
	static int created = 0;
	static int destroyed = 0;
	static void* last_call = nullptr;
	static void* static_call = reinterpret_cast<void*>(0x01);
	typedef void(*fptr)();
	struct x {
		x() { ++created; }
		x(const x&) { ++created; }
		x(x&&) { ++created; }
		x& operator=(const x&) { return *this; }
		x& operator=(x&&) { return *this; }
		void func() { last_call = static_cast<void*>(this); };
		~x() { ++destroyed; }
	};
	struct y {
		y() { ++created; }
		y(const x&) { ++created; }
		y(x&&) { ++created; }
		y& operator=(const x&) { return *this; }
		y& operator=(x&&) { return *this; }
		static void func() { last_call = static_call; };
		void operator()() { func(); }
		operator fptr () { return func; }
		~y() { ++destroyed; }
	};

	// stateful functors/member functions should always copy unless specified
	{
		created = 0;
		destroyed = 0;
		last_call = nullptr;
		{
			sol::state lua;
			x x1;
			lua.set_function("x1copy", &x::func, x1);
			lua.script("x1copy()");
			REQUIRE(created == 2);
			REQUIRE(destroyed == 0);
			REQUIRE_FALSE(last_call == &x1);

			lua.set_function("x1ref", &x::func, std::ref(x1));
			lua.script("x1ref()");
			REQUIRE(created == 2);
			REQUIRE(destroyed == 0);
			REQUIRE(last_call == &x1);
		}
		REQUIRE(created == 2);
		REQUIRE(destroyed == 2);
	}

	// things convertible to a static function should _never_ be forced to make copies
	// therefore, pass through untouched
	{
		created = 0;
		destroyed = 0;
		last_call = nullptr;
		{
			sol::state lua;
			y y1;
			lua.set_function("y1copy", y1);
			lua.script("y1copy()");
			REQUIRE(created == 1);
			REQUIRE(destroyed == 0);
			REQUIRE(last_call == static_call);

			last_call = nullptr;
			lua.set_function("y1ref", std::ref(y1));
			lua.script("y1ref()");
			REQUIRE(created == 1);
			REQUIRE(destroyed == 0);
			REQUIRE(last_call == static_call);
		}
		REQUIRE(created == 1);
		REQUIRE(destroyed == 1);
	}
}


TEST_CASE("functions/all-kinds", "Register all kinds of functions, make sure they all compile and work") {
	sol::state lua;

	struct test_1 {
		int a = 0xA;
		virtual int bark() {
			return a;
		}

		int bark_mem() {
			return a;
		}

		static std::tuple<int, int> x_bark(int num_value, test_1* a) {
			return std::tuple<int, int>(num_value * 2, a->a);
		}
	};

	struct test_2 {
		int a = 0xC;
		int bark() {
			return 20;
		}
	};

	struct inner {
		const int z = 5653;
	};

	struct nested {
		inner i;
	};

	auto a = []() { return 500; };
	auto b = [&]() { return 501; };
	auto c = [&]() { return 502; };
	auto d = []() { return 503; };

	lua.new_usertype<test_1>("test_1",
		"bark", sol::c_call<decltype(&test_1::bark_mem), &test_1::bark_mem>
		);
	lua.new_usertype<test_2>("test_2",
		"bark", sol::c_call<decltype(&test_2::bark), &test_2::bark>
		);
	test_2 t2;

	lua.set_function("a", a);
	lua.set_function("b", b);
	lua.set_function("c", std::ref(c));
	lua.set_function("d", std::ref(d));
	lua.set_function("f", &test_1::bark);
	lua.set_function("g", test_1::x_bark);
	lua.set_function("h", sol::c_call<decltype(&test_1::bark_mem), &test_1::bark_mem>);
	lua.set_function("i", &test_2::bark, test_2());
	lua.set_function("j", &test_2::a, test_2());
	lua.set_function("k", &test_2::a);
	lua.set_function("l", sol::c_call<decltype(&test_1::a), &test_1::a>);
	lua.set_function("m", &test_2::a, &t2);
	lua.set_function("n", sol::c_call<decltype(&non_overloaded), &non_overloaded>);

	lua.script(R"(
o1 = test_1.new()
o2 = test_2.new()
)");

	lua.script(R"(
ob = o1:bark()

A = a()
B = b()
C = c()
D = d()
F = f(o1)
G0, G1 = g(2, o1)
H = h(o1)
I = i(o1)
I = i(o1)
)");

	
	lua.script(R"(
J0 = j()
j(24)
J1 = j()
    )");

	lua.script(R"(
K0 = k(o2)
k(o2, 1024)
K1 = k(o2)
    )");

	lua.script(R"(
L0 = l(o1)
l(o1, 678)
L1 = l(o1)
    )");


	lua.script(R"(
M0 = m()
m(256)
M1 = m()
    )");

	lua.script(R"(
N = n(1, 2, 3)
    )");
	int ob, A, B, C, D, F, G0, G1, H, I, J0, J1, K0, K1, L0, L1, M0, M1, N;
	std::tie(ob, A, B, C, D, F, G0, G1, H, I, J0, J1, K0, K1, L0, L1, M0, M1, N)
		= lua.get<int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int>(
			"ob", "A", "B", "C", "D", "F", "G0", "G1", "H", "I", "J0", "J1", "K0", "K1", "L0", "L1", "M0", "M1", "N"
			);

	REQUIRE(ob == 0xA);

	REQUIRE(A == 500);
	REQUIRE(B == 501);
	REQUIRE(C == 502);
	REQUIRE(D == 503);

	REQUIRE(F == 0xA);
	REQUIRE(G0 == 4);
	REQUIRE(G1 == 0xA);
	REQUIRE(H == 0xA);
	REQUIRE(I == 20);

	REQUIRE(J0 == 0xC);
	REQUIRE(J1 == 24);

	REQUIRE(K0 == 0xC);
	REQUIRE(K1 == 1024);

	REQUIRE(L0 == 0xA);
	REQUIRE(L1 == 678);

	REQUIRE(M0 == 0xC);
	REQUIRE(M1 == 256);

	REQUIRE(N == 13);

	sol::tie(ob, A, B, C, D, F, G0, G1, H, I, J0, J1, K0, K1, L0, L1, M0, M1, N)
		= lua.get<int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int, int>(
			"ob", "A", "B", "C", "D", "F", "G0", "G1", "H", "I", "J0", "J1", "K0", "K1", "L0", "L1", "M0", "M1", "N"
			);

	REQUIRE(ob == 0xA);

	REQUIRE(A == 500);
	REQUIRE(B == 501);
	REQUIRE(C == 502);
	REQUIRE(D == 503);

	REQUIRE(F == 0xA);
	REQUIRE(G0 == 4);
	REQUIRE(G1 == 0xA);
	REQUIRE(H == 0xA);
	REQUIRE(I == 20);

	REQUIRE(J0 == 0xC);
	REQUIRE(J1 == 24);

	REQUIRE(K0 == 0xC);
	REQUIRE(K1 == 1024);

	REQUIRE(L0 == 0xA);
	REQUIRE(L1 == 678);

	REQUIRE(M0 == 0xC);
	REQUIRE(M1 == 256);

	REQUIRE(N == 13);

	// Work that compiler, WORK IT!
	lua.set("o", &test_1::bark);
	lua.set("p", test_1::x_bark);
	lua.set("q", sol::c_call<decltype(&test_1::bark_mem), &test_1::bark_mem>);
	lua.set("r", &test_2::a);
	lua.set("s", sol::readonly(&test_2::a));
	lua.set_function("t", sol::readonly(&test_2::a), test_2());
	lua.set_function("u", &nested::i, nested());
	lua.set("v", &nested::i);
	lua.set("nested", nested());
	lua.set("inner", inner());
	REQUIRE_THROWS(lua.script("s(o2, 2)"));
	REQUIRE_THROWS(lua.script("t(2)"));
	REQUIRE_THROWS(lua.script("u(inner)"));
	REQUIRE_THROWS(lua.script("v(nested, inner)"));
}

TEST_CASE("simple/call-with-parameters", "Lua function is called with a few parameters from C++") {
	sol::state lua;

	REQUIRE_NOTHROW(lua.script("function my_add(i, j, k) return i + j + k end"));
	auto f = lua.get<sol::function>("my_add");
	REQUIRE_NOTHROW(lua.script("function my_nothing(i, j, k) end"));
	auto fvoid = lua.get<sol::function>("my_nothing");
	int a;
	REQUIRE_NOTHROW(fvoid(1, 2, 3));
	REQUIRE_NOTHROW(a = f.call<int>(1, 2, 3));
	REQUIRE(a == 6);
	REQUIRE_THROWS(a = f(1, 2, "arf"));
}

TEST_CASE("simple/call-c++-function", "C++ function is called from lua") {
	sol::state lua;

	lua.set_function("plop_xyz", sep::plop_xyz);
	lua.script("x = plop_xyz(2, 6, 'hello')");

	REQUIRE(lua.get<int>("x") == 11);
}

TEST_CASE("simple/call-lambda", "A C++ lambda is exposed to lua and called") {
	sol::state lua;

	int a = 0;

	lua.set_function("foo", [&a] { a = 1; });

	lua.script("foo()");

	REQUIRE(a == 1);
}

TEST_CASE("advanced/get-and-call", "Checks for lambdas returning values after a get operation") {
	const static std::string lol = "lol", str = "str";
	const static std::tuple<int, float, double, std::string> heh_tuple = std::make_tuple(1, 6.28f, 3.14, std::string("heh"));
	sol::state lua;

	REQUIRE_NOTHROW(lua.set_function("a", [] { return 42; }));
	REQUIRE(lua.get<sol::function>("a").call<int>() == 42);

	REQUIRE_NOTHROW(lua.set_function("b", [] { return 42u; }));
	REQUIRE(lua.get<sol::function>("b").call<unsigned int>() == 42u);

	REQUIRE_NOTHROW(lua.set_function("c", [] { return 3.14; }));
	REQUIRE(lua.get<sol::function>("c").call<double>() == 3.14);

	REQUIRE_NOTHROW(lua.set_function("d", [] { return 6.28f; }));
	REQUIRE(lua.get<sol::function>("d").call<float>() == 6.28f);

	REQUIRE_NOTHROW(lua.set_function("e", [] { return "lol"; }));
	REQUIRE(lua.get<sol::function>("e").call<std::string>() == lol);

	REQUIRE_NOTHROW(lua.set_function("f", [] { return true; }));
	REQUIRE(lua.get<sol::function>("f").call<bool>());

	REQUIRE_NOTHROW(lua.set_function("g", [] { return std::string("str"); }));
	REQUIRE(lua.get<sol::function>("g").call<std::string>() == str);

	REQUIRE_NOTHROW(lua.set_function("h", [] {}));
	REQUIRE_NOTHROW(lua.get<sol::function>("h").call());

	REQUIRE_NOTHROW(lua.set_function("i", [] { return sol::nil; }));
	REQUIRE(lua.get<sol::function>("i").call<sol::nil_t>() == sol::nil);
	REQUIRE_NOTHROW(lua.set_function("j", [] { return std::make_tuple(1, 6.28f, 3.14, std::string("heh")); }));
	REQUIRE((lua.get<sol::function>("j").call<int, float, double, std::string>() == heh_tuple));
}

TEST_CASE("advanced/operator[]-call", "Checks for lambdas returning values using operator[]") {
	const static std::string lol = "lol", str = "str";
	const static std::tuple<int, float, double, std::string> heh_tuple = std::make_tuple(1, 6.28f, 3.14, std::string("heh"));
	sol::state lua;

	REQUIRE_NOTHROW(lua.set_function("a", [] { return 42; }));
	REQUIRE(lua["a"].call<int>() == 42);

	REQUIRE_NOTHROW(lua.set_function("b", [] { return 42u; }));
	REQUIRE(lua["b"].call<unsigned int>() == 42u);

	REQUIRE_NOTHROW(lua.set_function("c", [] { return 3.14; }));
	REQUIRE(lua["c"].call<double>() == 3.14);

	REQUIRE_NOTHROW(lua.set_function("d", [] { return 6.28f; }));
	REQUIRE(lua["d"].call<float>() == 6.28f);

	REQUIRE_NOTHROW(lua.set_function("e", [] { return "lol"; }));
	REQUIRE(lua["e"].call<std::string>() == lol);

	REQUIRE_NOTHROW(lua.set_function("f", [] { return true; }));
	REQUIRE(lua["f"].call<bool>());

	REQUIRE_NOTHROW(lua.set_function("g", [] { return std::string("str"); }));
	REQUIRE(lua["g"].call<std::string>() == str);

	REQUIRE_NOTHROW(lua.set_function("h", [] {}));
	REQUIRE_NOTHROW(lua["h"].call());

	REQUIRE_NOTHROW(lua.set_function("i", [] { return sol::nil; }));
	REQUIRE(lua["i"].call<sol::nil_t>() == sol::nil);
	REQUIRE_NOTHROW(lua.set_function("j", [] { return std::make_tuple(1, 6.28f, 3.14, std::string("heh")); }));
	REQUIRE((lua["j"].call<int, float, double, std::string>() == heh_tuple));
}

TEST_CASE("advanced/call-lambdas", "A C++ lambda is exposed to lua and called") {
	sol::state lua;

	int x = 0;
	lua.set_function("set_x", [&](int new_x) {
		x = new_x;
		return 0;
	});

	lua.script("set_x(9)");
	REQUIRE(x == 9);
}

TEST_CASE("advanced/call-referenced_obj", "A C++ object is passed by pointer/reference_wrapper to lua and invoked") {
	sol::state lua;

	int x = 0;
	auto objx = [&](int new_x) {
		x = new_x;
		return 0;
	};
	lua.set_function("set_x", std::ref(objx));

	int y = 0;
	auto objy = [&](int new_y) {
		y = new_y;
		return std::tuple<int, int>(0, 0);
	};
	lua.set_function("set_y", &decltype(objy)::operator(), std::ref(objy));

	lua.script("set_x(9)");
	lua.script("set_y(9)");
	REQUIRE(x == 9);
	REQUIRE(y == 9);
}

TEST_CASE("functions/tie", "make sure advanced syntax with 'tie' works") {
	sol::state lua;

	lua.script(R"(function f () 
    return 1, 2, 3 
end)");
	sol::function f = lua["f"];

	int a, b, c;
	sol::tie(a, b, c) = f();
	REQUIRE(a == 1);
	REQUIRE(b == 2);
	REQUIRE(c == 3);
}

TEST_CASE("functions/variadic_args", "Check to see we can receive multiple arguments through a variadic") {
	struct structure {
		int x;
		bool b;
	};

	sol::state lua;
	lua.open_libraries(sol::lib::base);
	lua.set_function("v", [](sol::this_state, sol::variadic_args va) -> structure {
		int r = 0;
		for (auto v : va) {
			int value = v;
			r += value;
		}
		return{ r, r > 200 };
	});

	lua.script("x = v(25, 25)");
	lua.script("x2 = v(25, 25, 100, 50, 250, 150)");
	lua.script("x3 = v(1, 2, 3, 4, 5, 6)");

	structure& lx = lua["x"];
	structure& lx2 = lua["x2"];
	structure& lx3 = lua["x3"];
	REQUIRE(lx.x == 50);
	REQUIRE(lx2.x == 600);
	REQUIRE(lx3.x == 21);
	REQUIRE_FALSE(lx.b);
	REQUIRE(lx2.b);
	REQUIRE_FALSE(lx3.b);
}

TEST_CASE("functions/required_and_variadic_args", "Check if a certain number of arguments can still be required even when using variadic_args") {
	sol::state lua;
	lua.set_function("v", 
		[](sol::this_state, sol::variadic_args, int, int) {
		}
	);
	REQUIRE_NOTHROW(lua.script("v(20, 25, 30)"));
	REQUIRE_NOTHROW(lua.script("v(20, 25)"));
#ifndef SOL_LUAJIT
	REQUIRE_THROWS(lua.script("v(20)"));
#endif // LuaJIT has problems with exceptions, as fucking usual
}

TEST_CASE("functions/overloading", "Check if overloading works properly for regular set function syntax") {
	sol::state lua;
	lua.open_libraries(sol::lib::base);

	lua.set_function("func_1", func_1);
	lua.set_function("func", sol::overload(func_2, func_3, func_1, func_1s));

	const std::string string_bark = "string: bark";

	REQUIRE_NOTHROW(lua.script(
		"a = func(1)\n"
		"b = func('bark')\n"
		"c = func(1,2)\n"
		"func(1,2,3)\n"
	));

	REQUIRE((lua["a"] == 1));
	REQUIRE((lua["b"] == string_bark));
	REQUIRE((lua["c"] == 2));

	REQUIRE_THROWS(lua.script("func(1,2,'meow')"));
}

TEST_CASE("overloading/c_call", "Make sure that overloading works with c_call functionality") {
	sol::state lua;
	lua.set("f", sol::c_call<sol::wrap<decltype(&f1), &f1>, sol::wrap<decltype(&f2), &f2>, sol::wrap<decltype(&fer::f3), &fer::f3>>);
	lua.set("g", sol::c_call<sol::wrap<decltype(&f1), &f1>>);
	lua.set("h", sol::c_call<decltype(&f2), &f2>);
	lua.set("obj", fer());

	lua.script("r1 = f(1)");
	lua.script("r2 = f(1, 2)");
	lua.script("r3 = f(obj, 1, 2)");
	lua.script("r4 = g(1)");
	lua.script("r5 = h(1, 2)");

	int r1 = lua["r1"];
	int r2 = lua["r2"];
	double r3 = lua["r3"];
	int r4 = lua["r4"];
	int r5 = lua["r5"];

	REQUIRE(r1 == 32);
	REQUIRE(r2 == 1);
	REQUIRE(r3 == 2.5);
	REQUIRE(r4 == 32);
	REQUIRE(r5 == 1);
}

TEST_CASE("functions/stack-protect", "make sure functions don't impede on the stack") {
	//setup sol/lua
	sol::state lua;
	lua.open_libraries(sol::lib::base, sol::lib::string);

	lua.script("function ErrorHandler(msg) print('Lua created error msg : ' ..  msg) return msg end");
	lua.script("function stringtest(a) if a == nil then error('fuck') end print('Lua recieved content : ' .. a) return a end");

	// test normal function
	{
		sol::stack_guard normalsg(lua);
		std::string str = lua["stringtest"]("normal test");
		INFO("Back in C++, direct call result is : " << str);
	}

	//test protected_function
	sol::protected_function Stringtest(lua["stringtest"]);
	Stringtest.error_handler = lua["ErrorHandler"];
	sol::stack_guard sg(lua);
	{
		sol::protected_function_result stringresult = Stringtest("protected test");
		REQUIRE(stringresult.valid());
		std::string s = stringresult;
		INFO("Back in C++, protected result is : " << s);
	}
	REQUIRE(sg.check_stack());

	//test optional
	{
		sol::stack_guard opsg(lua);
		sol::optional<std::string> opt_result = Stringtest("optional test");
		REQUIRE(opsg.check_stack());
		if (opt_result)
		{
			std::string s = opt_result.value();
			INFO("Back in C++, opt_result is : " << s);
		}
		else
		{
			INFO("opt_result failed");
		}
	}
	REQUIRE(sg.check_stack());

	{
		sol::protected_function_result errresult = Stringtest(sol::nil);
		REQUIRE_FALSE(errresult.valid());
		sol::error err = errresult;
		std::string msg = err.what();
		INFO("error :" << msg);
	}
	REQUIRE(sg.check_stack());
}
