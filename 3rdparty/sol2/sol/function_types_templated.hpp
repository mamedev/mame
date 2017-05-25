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

#ifndef SOL_FUNCTION_TYPES_TEMPLATED_HPP
#define SOL_FUNCTION_TYPES_TEMPLATED_HPP

#include "call.hpp"

namespace sol {
	namespace function_detail {
		template <typename F, F fx>
		inline int call_wrapper_variable(std::false_type, lua_State* L) {
			typedef meta::bind_traits<meta::unqualified_t<F>> traits_type;
			typedef typename traits_type::args_list args_list;
			typedef meta::tuple_types<typename traits_type::return_type> return_type;
			return stack::call_into_lua(return_type(), args_list(), L, 1, fx);
		}

		template <typename R, typename V, V, typename T>
		inline int call_set_assignable(std::false_type, T&&, lua_State* L) {
			return luaL_error(L, "cannot write to this type: copy assignment/constructor not available");
		}

		template <typename R, typename V, V variable, typename T>
		inline int call_set_assignable(std::true_type, lua_State* L, T&& mem) {
			(mem.*variable) = stack::get<R>(L, 2);
			return 0;
		}

		template <typename R, typename V, V, typename T>
		inline int call_set_variable(std::false_type, lua_State* L, T&&) {
			return luaL_error(L, "cannot write to a const variable");
		}

		template <typename R, typename V, V variable, typename T>
		inline int call_set_variable(std::true_type, lua_State* L, T&& mem) {
			return call_set_assignable<R, V, variable>(std::is_assignable<std::add_lvalue_reference_t<R>, R>(), L, std::forward<T>(mem));
		}

		template <typename V, V variable>
		inline int call_wrapper_variable(std::true_type, lua_State* L) {
			typedef meta::bind_traits<meta::unqualified_t<V>> traits_type;
			typedef typename traits_type::object_type T;
			typedef typename traits_type::return_type R;
			auto& mem = stack::get<T>(L, 1);
			switch (lua_gettop(L)) {
			case 1: {
				decltype(auto) r = (mem.*variable);
				stack::push_reference(L, std::forward<decltype(r)>(r));
				return 1; }
			case 2:
				return call_set_variable<R, V, variable>(meta::neg<std::is_const<R>>(), L, mem);
			default:
				return luaL_error(L, "incorrect number of arguments to member variable function call");
			}
		}

		template <typename F, F fx>
		inline int call_wrapper_function(std::false_type, lua_State* L) {
			return call_wrapper_variable<F, fx>(std::is_member_object_pointer<F>(), L);
		}

		template <typename F, F fx>
		inline int call_wrapper_function(std::true_type, lua_State* L) {
			return call_detail::call_wrapped<void, false, false>(L, fx);
		}

		template <typename F, F fx>
		int call_wrapper_entry(lua_State* L) {
			return call_wrapper_function<F, fx>(std::is_member_function_pointer<meta::unqualified_t<F>>(), L);
		}

		template <typename... Fxs>
		struct c_call_matcher {
			template <typename Fx, std::size_t I, typename R, typename... Args>
			int operator()(types<Fx>, index_value<I>, types<R>, types<Args...>, lua_State* L, int, int) const {
				typedef meta::at_in_pack_t<I, Fxs...> target;
				return target::call(L);
			}
		};

	} // function_detail

	template <typename F, F fx>
	inline int c_call(lua_State* L) {
#ifdef __clang__
		return detail::trampoline(L, function_detail::call_wrapper_entry<F, fx>);
#else
		return detail::static_trampoline<(&function_detail::call_wrapper_entry<F, fx>)>(L);
#endif // fuck you clang :c
	}

	template <typename F, F f>
	struct wrap {
		typedef F type;

		static int call(lua_State* L) {
			return c_call<type, f>(L);
		}
	};

	template <typename... Fxs>
	inline int c_call(lua_State* L) {
		if (sizeof...(Fxs) < 2) {
			return meta::at_in_pack_t<0, Fxs...>::call(L);
		}
		else {
			return call_detail::overload_match_arity<typename Fxs::type...>(function_detail::c_call_matcher<Fxs...>(), L, lua_gettop(L), 1);
		}
	}

} // sol

#endif // SOL_FUNCTION_TYPES_TEMPLATED_HPP
