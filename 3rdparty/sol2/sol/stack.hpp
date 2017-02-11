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

#ifndef SOL_STACK_HPP
#define SOL_STACK_HPP

#include "stack_core.hpp"
#include "stack_reference.hpp"
#include "stack_check.hpp"
#include "stack_get.hpp"
#include "stack_check_get.hpp"
#include "stack_push.hpp"
#include "stack_pop.hpp"
#include "stack_field.hpp"
#include "stack_probe.hpp"
#include <cstring>
#include <array>

namespace sol {
	namespace stack {
		namespace stack_detail {
			template<typename T>
			inline int push_as_upvalues(lua_State* L, T& item) {
				typedef std::decay_t<T> TValue;
				const static std::size_t itemsize = sizeof(TValue);
				const static std::size_t voidsize = sizeof(void*);
				const static std::size_t voidsizem1 = voidsize - 1;
				const static std::size_t data_t_count = (sizeof(TValue) + voidsizem1) / voidsize;
				typedef std::array<void*, data_t_count> data_t;

				data_t data{ {} };
				std::memcpy(&data[0], std::addressof(item), itemsize);
				int pushcount = 0;
				for (auto&& v : data) {
					pushcount += push(L, lightuserdata_value(v));
				}
				return pushcount;
			}

			template<typename T>
			inline std::pair<T, int> get_as_upvalues(lua_State* L, int index = 1) {
				const static std::size_t data_t_count = (sizeof(T) + (sizeof(void*) - 1)) / sizeof(void*);
				typedef std::array<void*, data_t_count> data_t;
				data_t voiddata{ {} };
				for (std::size_t i = 0, d = 0; d < sizeof(T); ++i, d += sizeof(void*)) {
					voiddata[i] = get<lightuserdata_value>(L, upvalue_index(index++));
				}
				return std::pair<T, int>(*reinterpret_cast<T*>(static_cast<void*>(voiddata.data())), index);
			}

			struct evaluator {
				template <typename Fx, typename... Args>
				static decltype(auto) eval(types<>, std::index_sequence<>, lua_State*, int, record&, Fx&& fx, Args&&... args) {
					return std::forward<Fx>(fx)(std::forward<Args>(args)...);
				}
				
				template <typename Fx, typename Arg, typename... Args, std::size_t I, std::size_t... Is, typename... FxArgs>
				static decltype(auto) eval(types<Arg, Args...>, std::index_sequence<I, Is...>, lua_State* L, int start, record& tracking, Fx&& fx, FxArgs&&... fxargs) {
					return eval(types<Args...>(), std::index_sequence<Is...>(), L, start, tracking, std::forward<Fx>(fx), std::forward<FxArgs>(fxargs)..., stack_detail::unchecked_get<Arg>(L, start + tracking.used, tracking));
				}
			};

			template <bool checkargs = default_check_arguments, std::size_t... I, typename R, typename... Args, typename Fx, typename... FxArgs, typename = std::enable_if_t<!std::is_void<R>::value>>
			inline decltype(auto) call(types<R>, types<Args...> ta, std::index_sequence<I...> tai, lua_State* L, int start, Fx&& fx, FxArgs&&... args) {
#ifndef _MSC_VER
				static_assert(meta::all<meta::is_not_move_only<Args>...>::value, "One of the arguments being bound is a move-only type, and it is not being taken by reference: this will break your code. Please take a reference and std::move it manually if this was your intention.");
#endif // This compiler make me so fucking sad
				multi_check<checkargs, Args...>(L, start, type_panic);
				record tracking{};
				return evaluator{}.eval(ta, tai, L, start, tracking, std::forward<Fx>(fx), std::forward<FxArgs>(args)...);
			}

			template <bool checkargs = default_check_arguments, std::size_t... I, typename... Args, typename Fx, typename... FxArgs>
			inline void call(types<void>, types<Args...> ta, std::index_sequence<I...> tai, lua_State* L, int start, Fx&& fx, FxArgs&&... args) {
#ifndef _MSC_VER
				static_assert(meta::all<meta::is_not_move_only<Args>...>::value, "One of the arguments being bound is a move-only type, and it is not being taken by reference: this will break your code. Please take a reference and std::move it manually if this was your intention.");
#endif // This compiler make me so fucking sad
				multi_check<checkargs, Args...>(L, start, type_panic);
				record tracking{};
				evaluator{}.eval(ta, tai, L, start, tracking, std::forward<Fx>(fx), std::forward<FxArgs>(args)...);
			}
		} // stack_detail

		template <typename T>
		int set_ref(lua_State* L, T&& arg, int tableindex = -2) {
			push(L, std::forward<T>(arg));
			return luaL_ref(L, tableindex);
		}

		inline void remove(lua_State* L, int index, int count) {
			if (count < 1)
				return;
			int top = lua_gettop(L);
			if (index == -1 || top == index) {
				// Slice them right off the top
				lua_pop(L, static_cast<int>(count));
				return;
			}

			// Remove each item one at a time using stack operations
			// Probably slower, maybe, haven't benchmarked,
			// but necessary
			if (index < 0) {
				index = lua_gettop(L) + (index + 1);
			}
			int last = index + count;
			for (int i = index; i < last; ++i) {
				lua_remove(L, index);
			}
		}

		template <bool check_args = stack_detail::default_check_arguments, typename R, typename... Args, typename Fx, typename... FxArgs, typename = std::enable_if_t<!std::is_void<R>::value>>
		inline decltype(auto) call(types<R> tr, types<Args...> ta, lua_State* L, int start, Fx&& fx, FxArgs&&... args) {
			typedef std::make_index_sequence<sizeof...(Args)> args_indices;
			return stack_detail::call<check_args>(tr, ta, args_indices(), L, start, std::forward<Fx>(fx), std::forward<FxArgs>(args)...);
		}

		template <bool check_args = stack_detail::default_check_arguments, typename R, typename... Args, typename Fx, typename... FxArgs, typename = std::enable_if_t<!std::is_void<R>::value>>
		inline decltype(auto) call(types<R> tr, types<Args...> ta, lua_State* L, Fx&& fx, FxArgs&&... args) {
			return call<check_args>(tr, ta, L, 1, std::forward<Fx>(fx), std::forward<FxArgs>(args)...);
		}

		template <bool check_args = stack_detail::default_check_arguments, typename... Args, typename Fx, typename... FxArgs>
		inline void call(types<void> tr, types<Args...> ta, lua_State* L, int start, Fx&& fx, FxArgs&&... args) {
			typedef std::make_index_sequence<sizeof...(Args)> args_indices;
			stack_detail::call<check_args>(tr, ta, args_indices(), L, start, std::forward<Fx>(fx), std::forward<FxArgs>(args)...);
		}

		template <bool check_args = stack_detail::default_check_arguments, typename... Args, typename Fx, typename... FxArgs>
		inline void call(types<void> tr, types<Args...> ta, lua_State* L, Fx&& fx, FxArgs&&... args) {
			call<check_args>(tr, ta, L, 1, std::forward<Fx>(fx), std::forward<FxArgs>(args)...);
		}

		template <bool check_args = stack_detail::default_check_arguments, typename R, typename... Args, typename Fx, typename... FxArgs, typename = std::enable_if_t<!std::is_void<R>::value>>
		inline decltype(auto) call_from_top(types<R> tr, types<Args...> ta, lua_State* L, Fx&& fx, FxArgs&&... args) {
			return call<check_args>(tr, ta, L, static_cast<int>(lua_gettop(L) - sizeof...(Args)), std::forward<Fx>(fx), std::forward<FxArgs>(args)...);
		}

		template <bool check_args = stack_detail::default_check_arguments, typename... Args, typename Fx, typename... FxArgs>
		inline void call_from_top(types<void> tr, types<Args...> ta, lua_State* L, Fx&& fx, FxArgs&&... args) {
			call<check_args>(tr, ta, L, static_cast<int>(lua_gettop(L) - sizeof...(Args)), std::forward<Fx>(fx), std::forward<FxArgs>(args)...);
		}

		template<bool check_args = stack_detail::default_check_arguments, typename... Args, typename Fx, typename... FxArgs>
		inline int call_into_lua(types<void> tr, types<Args...> ta, lua_State* L, int start, Fx&& fx, FxArgs&&... fxargs) {
			call<check_args>(tr, ta, L, start, std::forward<Fx>(fx), std::forward<FxArgs>(fxargs)...);
			lua_settop(L, 0);
			return 0;
		}

		template<bool check_args = stack_detail::default_check_arguments, typename Ret0, typename... Ret, typename... Args, typename Fx, typename... FxArgs, typename = std::enable_if_t<meta::neg<std::is_void<Ret0>>::value>>
		inline int call_into_lua(types<Ret0, Ret...>, types<Args...> ta, lua_State* L, int start, Fx&& fx, FxArgs&&... fxargs) {
			decltype(auto) r = call<check_args>(types<meta::return_type_t<Ret0, Ret...>>(), ta, L, start, std::forward<Fx>(fx), std::forward<FxArgs>(fxargs)...);
			lua_settop(L, 0);
			return push_reference(L, std::forward<decltype(r)>(r));
		}

		template<bool check_args = stack_detail::default_check_arguments, typename Fx, typename... FxArgs>
		inline int call_lua(lua_State* L, int start, Fx&& fx, FxArgs&&... fxargs) {
			typedef lua_bind_traits<meta::unqualified_t<Fx>> traits_type;
			typedef typename traits_type::args_list args_list;
			typedef typename traits_type::returns_list returns_list;
			return call_into_lua(returns_list(), args_list(), L, start, std::forward<Fx>(fx), std::forward<FxArgs>(fxargs)...);
		}

		inline call_syntax get_call_syntax(lua_State* L, const std::string& key, int index) {
			if (lua_gettop(L) == 0) {
				return call_syntax::dot;
			}
			luaL_getmetatable(L, key.c_str());
			auto pn = pop_n(L, 1);
			if (lua_compare(L, -1, index, LUA_OPEQ) != 1) {
				return call_syntax::dot;
			}
			return call_syntax::colon;
		}

		inline void script(lua_State* L, const std::string& code) {
			if (luaL_dostring(L, code.c_str())) {
				lua_error(L);
			}
		}

		inline void script_file(lua_State* L, const std::string& filename) {
			if (luaL_dofile(L, filename.c_str())) {
				lua_error(L);
			}
		}

		inline void luajit_exception_handler(lua_State* L, int(*handler)(lua_State*, lua_CFunction) = detail::c_trampoline) {
#ifdef SOL_LUAJIT
			lua_pushlightuserdata(L, (void*)handler);
			auto pn = pop_n(L, 1);
			luaJIT_setmode(L, -1, LUAJIT_MODE_WRAPCFUNC | LUAJIT_MODE_ON);
#else
			(void)L;
			(void)handler;
#endif
		}

		inline void luajit_exception_off(lua_State* L) {
#ifdef SOL_LUAJIT
			luaJIT_setmode(L, -1, LUAJIT_MODE_WRAPCFUNC | LUAJIT_MODE_OFF);
#else
			(void)L;
#endif
		}
	} // stack
} // sol

#endif // SOL_STACK_HPP
