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

#ifndef SOL_THREAD_HPP
#define SOL_THREAD_HPP

#include "reference.hpp"
#include "stack.hpp"

namespace sol {
	struct lua_thread_state {
		lua_State* L;
		operator lua_State* () const {
			return L;
		}
		lua_State* operator-> () const {
			return L;
		}
	};

	namespace stack {

		template <>
		struct pusher<lua_thread_state> {
			int push(lua_State*, lua_thread_state lts) {
				lua_pushthread(lts.L);
				return 1;
			}
		};

		template <>
		struct getter<lua_thread_state> {
			lua_thread_state get(lua_State* L, int index, record& tracking) {
				tracking.use(1);
				lua_thread_state lts{ lua_tothread(L, index) };
				return lts;
			}
		};

		template <>
		struct check_getter<lua_thread_state> {
			template <typename Handler>
			optional<lua_thread_state> get(lua_State* L, int index, Handler&& handler, record& tracking) {
				lua_thread_state lts{ lua_tothread(L, index) };
				if (lts.L == nullptr) {
					handler(L, index, type::thread, type_of(L, index));
					return nullopt;
				}
				tracking.use(1);
				return lts;
			}
		};

	}

#if SOL_LUA_VERSION < 502
	inline lua_State* main_thread(lua_State*, lua_State* backup_if_unsupported = nullptr) {
		return backup_if_unsupported;
	}
#else
	inline lua_State* main_thread(lua_State* L, lua_State* = nullptr) {
		lua_rawgeti(L, LUA_REGISTRYINDEX, LUA_RIDX_MAINTHREAD);
		lua_thread_state s = stack::pop<lua_thread_state>(L);
		return s.L;
	}
#endif // Lua 5.2+ has the main thread getter

	class thread : public reference {
	public:
		thread() noexcept = default;
		thread(const thread&) = default;
		thread(thread&&) = default;
		template <typename T, meta::enable<meta::neg<std::is_same<meta::unqualified_t<T>, thread>>, std::is_base_of<reference, meta::unqualified_t<T>>> = meta::enabler>
		thread(T&& r) : reference(std::forward<T>(r)) {}
		thread(const stack_reference& r) : thread(r.lua_state(), r.stack_index()) {};
		thread(stack_reference&& r) : thread(r.lua_state(), r.stack_index()) {};
		thread& operator=(const thread&) = default;
		thread& operator=(thread&&) = default;
		template <typename T, meta::enable<meta::neg<std::is_integral<meta::unqualified_t<T>>>, meta::neg<std::is_same<T, ref_index>>> = meta::enabler>
		thread(lua_State* L, T&& r) : thread(L, sol::ref_index(r.registry_index())) {}
		thread(lua_State* L, int index = -1) : reference(L, index) {
#ifdef SOL_CHECK_ARGUMENTS
			type_assert(L, index, type::thread);
#endif // Safety
		}
		thread(lua_State* L, ref_index index) : reference(L, index) {
#ifdef SOL_CHECK_ARGUMENTS
			auto pp = stack::push_pop(*this);
			type_assert(L, -1, type::thread);
#endif // Safety
		}
		thread(lua_State* L, lua_State* actualthread) : thread(L, lua_thread_state{ actualthread }) {}
		thread(lua_State* L, sol::this_state actualthread) : thread(L, lua_thread_state{ actualthread.L }) {}
		thread(lua_State* L, lua_thread_state actualthread) : reference(L, -stack::push(L, actualthread)) {
#ifdef SOL_CHECK_ARGUMENTS
			type_assert(L, -1, type::thread);
#endif // Safety
			lua_pop(L, 1);
		}

		state_view state() const {
			return state_view(this->thread_state());
		}

		bool is_main_thread() const {
			int ismainthread = lua_pushthread(this->thread_state());
			lua_pop(this->thread_state(), 1);
			return ismainthread == 1;
		}

		lua_State* thread_state() const {
			auto pp = stack::push_pop(*this);
			lua_State* lthread = lua_tothread(lua_state(), -1);
			return lthread;
		}

		thread_status status() const {
			lua_State* lthread = thread_state();
			thread_status lstat = static_cast<thread_status>(lua_status(lthread));
			if (lstat != thread_status::ok && lua_gettop(lthread) == 0) {
				// No thing on the thread's stack means its dead
				return thread_status::dead;
			}
			return lstat;
		}

		thread create() {
			return create(lua_state());
		}

		static thread create(lua_State* L) {
			lua_newthread(L);
			thread result(L);
			lua_pop(L, 1);
			return result;
		}
	};
} // sol

#endif // SOL_THREAD_HPP
