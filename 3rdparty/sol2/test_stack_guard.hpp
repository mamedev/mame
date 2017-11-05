#pragma once

struct test_stack_guard {
	lua_State* L;
	int& begintop;
	int& endtop;
	test_stack_guard(lua_State* L, int& begintop, int& endtop) : L(L), begintop(begintop), endtop(endtop) {
		begintop = lua_gettop(L);
	}
	~test_stack_guard() { endtop = lua_gettop(L); }
};
