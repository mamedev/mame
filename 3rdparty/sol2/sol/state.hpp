// The MIT License (MIT) 

// Copyright (c) 2013-2016 Rapptz, ThePhD and contributors

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef SOL_STATE_HPP
#define SOL_STATE_HPP

#include "state_view.hpp"

namespace sol {
	inline int default_at_panic(lua_State* L) {
#ifdef SOL_NO_EXCEPTIONS
		(void)L;
		return -1;
#else
		const char* message = lua_tostring(L, -1);
		if (message) {
			std::string err = message;
			lua_pop(L, 1);
			throw error(err);
		}
		throw error(std::string("An unexpected error occurred and forced the lua state to call atpanic"));
#endif
	}

	inline int default_error_handler(lua_State*L) {
		using namespace sol;
		std::string msg = "An unknown error has triggered the default error handler";
		optional<string_detail::string_shim> maybetopmsg = stack::check_get<string_detail::string_shim>(L, 1);
		if (maybetopmsg) {
			const string_detail::string_shim& topmsg = maybetopmsg.value();
			msg.assign(topmsg.c_str(), topmsg.size());
		}
		luaL_traceback(L, L, msg.c_str(), 1);
		optional<string_detail::string_shim> maybetraceback = stack::check_get<string_detail::string_shim>(L, -1);
		if (maybetraceback) {
			const string_detail::string_shim& traceback = maybetraceback.value();
			msg.assign(traceback.c_str(), traceback.size());
		}
		return stack::push(L, msg);
	}


	class state : private std::unique_ptr<lua_State, void(*)(lua_State*)>, public state_view {
	private:
		typedef std::unique_ptr<lua_State, void(*)(lua_State*)> unique_base;
	public:
		state(lua_CFunction panic = default_at_panic) : unique_base(luaL_newstate(), lua_close),
			state_view(unique_base::get()) {
			set_panic(panic);
			stack::luajit_exception_handler(unique_base::get());
		}

		state(lua_CFunction panic, lua_Alloc alfunc, void* alpointer = nullptr) : unique_base(lua_newstate(alfunc, alpointer), lua_close),
			state_view(unique_base::get()) {
			set_panic(panic);
			sol::protected_function::set_default_handler(sol::object(lua_state(), in_place, default_error_handler));
			stack::luajit_exception_handler(unique_base::get());
		}

		state(const state&) = delete;
		state(state&&) = default;
		state& operator=(const state&) = delete;
		state& operator=(state&&) = default;

		using state_view::get;

		~state() {
			auto& handler = protected_function::get_default_handler();
			if (handler.lua_state() == this->lua_state()) {
				protected_function::set_default_handler(reference());
			}
		}
	};
} // sol

#endif // SOL_STATE_HPP
