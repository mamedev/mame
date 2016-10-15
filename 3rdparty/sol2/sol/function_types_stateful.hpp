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

#ifndef SOL_FUNCTION_TYPES_STATEFUL_HPP
#define SOL_FUNCTION_TYPES_STATEFUL_HPP

#include "function_types_core.hpp"

namespace sol {
	namespace function_detail {
		template<typename Func>
		struct functor_function {
			typedef meta::unwrapped_t<meta::unqualified_t<Func>> Function;
			typedef decltype(&Function::operator()) function_type;
			typedef meta::function_return_t<function_type> return_type;
			typedef meta::function_args_t<function_type> args_lists;
			Function fx;

			template<typename... Args>
			functor_function(Function f, Args&&... args) : fx(std::move(f), std::forward<Args>(args)...) {}

			int call(lua_State* L) {
				return call_detail::call_wrapped<void, true, false>(L, fx);
			}

			int operator()(lua_State* L) {
				auto f = [&](lua_State* L) -> int { return this->call(L); };
				return detail::trampoline(L, f);
			}
		};

		template<typename T, typename Function>
		struct member_function {
			typedef std::remove_pointer_t<std::decay_t<Function>> function_type;
			typedef meta::function_return_t<function_type> return_type;
			typedef meta::function_args_t<function_type> args_lists;
			function_type invocation;
			T member;

			template<typename... Args>
			member_function(function_type f, Args&&... args) : invocation(std::move(f)), member(std::forward<Args>(args)...) {}

			int call(lua_State* L) {
				return call_detail::call_wrapped<T, true, false, -1>(L, invocation, detail::unwrap(detail::deref(member)));
			}

			int operator()(lua_State* L) {
				auto f = [&](lua_State* L) -> int { return this->call(L); };
				return detail::trampoline(L, f);
			}
		};

		template<typename T, typename Function>
		struct member_variable {
			typedef std::remove_pointer_t<std::decay_t<Function>> function_type;
			typedef typename meta::bind_traits<function_type>::return_type return_type;
			typedef typename meta::bind_traits<function_type>::args_list args_lists;
			function_type var;
			T member;
			typedef std::add_lvalue_reference_t<meta::unwrapped_t<std::remove_reference_t<decltype(detail::deref(member))>>> M;

			template<typename... Args>
			member_variable(function_type v, Args&&... args) : var(std::move(v)), member(std::forward<Args>(args)...) {}

			int call(lua_State* L) {
				M mem = detail::unwrap(detail::deref(member));
				switch (lua_gettop(L)) {
				case 0:
					return call_detail::call_wrapped<T, true, false, -1>(L, var, mem);
				case 1:
					return call_detail::call_wrapped<T, false, false, -1>(L, var, mem);
				default:
					return luaL_error(L, "sol: incorrect number of arguments to member variable function");
				}
			}

			int operator()(lua_State* L) {
				auto f = [&](lua_State* L) -> int { return this->call(L); };
				return detail::trampoline(L, f);
			}
		};
	} // function_detail
} // sol

#endif // SOL_FUNCTION_TYPES_STATEFUL_HPP
