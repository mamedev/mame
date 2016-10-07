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

#ifndef SOL_FUNCTION_TYPES_STATELESS_HPP
#define SOL_FUNCTION_TYPES_STATELESS_HPP

#include "stack.hpp"

namespace sol {
	namespace function_detail {
		template<typename Function>
		struct upvalue_free_function {
			typedef std::remove_pointer_t<std::decay_t<Function>> function_type;
			typedef lua_bind_traits<function_type> traits_type;

			static int real_call(lua_State* L) {
				auto udata = stack::stack_detail::get_as_upvalues<function_type*>(L);
				function_type* fx = udata.first;
				return call_detail::call_wrapped<void, true, false>(L, fx);
			}

			static int call(lua_State* L) {
				return detail::static_trampoline<(&real_call)>(L);
			}

			int operator()(lua_State* L) {
				return call(L);
			}
		};

		template<typename T, typename Function>
		struct upvalue_member_function {
			typedef std::remove_pointer_t<std::decay_t<Function>> function_type;
			typedef lua_bind_traits<function_type> traits_type;

			static int real_call(lua_State* L) {
				// Layout:
				// idx 1...n: verbatim data of member function pointer
				// idx n + 1: is the object's void pointer
				// We don't need to store the size, because the other side is templated
				// with the same member function pointer type
				auto memberdata = stack::stack_detail::get_as_upvalues<function_type>(L, 1);
				auto objdata = stack::stack_detail::get_as_upvalues<T*>(L, memberdata.second);
				function_type& memfx = memberdata.first;
				auto& item = *objdata.first;
				return call_detail::call_wrapped<T, true, false, -1>(L, memfx, item);
			}

			static int call(lua_State* L) {
				return detail::static_trampoline<(&real_call)>(L);
			}

			int operator()(lua_State* L) {
				return call(L);
			}
		};

		template<typename T, typename Function>
		struct upvalue_member_variable {
			typedef std::remove_pointer_t<std::decay_t<Function>> function_type;
			typedef lua_bind_traits<function_type> traits_type;

			static int real_call(lua_State* L) {
				// Layout:
				// idx 1...n: verbatim data of member variable pointer
				// idx n + 1: is the object's void pointer
				// We don't need to store the size, because the other side is templated
				// with the same member function pointer type
				auto memberdata = stack::stack_detail::get_as_upvalues<function_type>(L, 1);
				auto objdata = stack::stack_detail::get_as_upvalues<T*>(L, memberdata.second);
				auto& mem = *objdata.first;
				function_type& var = memberdata.first;
				switch (lua_gettop(L)) {
				case 0:
					return call_detail::call_wrapped<T, true, false, -1>(L, var, mem);
				case 1:
					return call_detail::call_wrapped<T, false, false, -1>(L, var, mem);
				default:
					return luaL_error(L, "sol: incorrect number of arguments to member variable function");
				}
			}

			static int call(lua_State* L) {
				return detail::static_trampoline<(&real_call)>(L);
			}

			int operator()(lua_State* L) {
				return call(L);
			}
		};

		template<typename T, typename Function>
		struct upvalue_this_member_function {
			typedef std::remove_pointer_t<std::decay_t<Function>> function_type;
			typedef lua_bind_traits<function_type> traits_type;

			static int real_call(lua_State* L) {
				// Layout:
				// idx 1...n: verbatim data of member variable pointer
				auto memberdata = stack::stack_detail::get_as_upvalues<function_type>(L, 1);
				function_type& memfx = memberdata.first;
				return call_detail::call_wrapped<T, false, false>(L, memfx);
			}

			static int call(lua_State* L) {
				return detail::static_trampoline<(&real_call)>(L);
			}

			int operator()(lua_State* L) {
				return call(L);
			}
		};

		template<typename T, typename Function>
		struct upvalue_this_member_variable {
			typedef std::remove_pointer_t<std::decay_t<Function>> function_type;
			typedef lua_bind_traits<function_type> traits_type;

			static int real_call(lua_State* L) {
				// Layout:
				// idx 1...n: verbatim data of member variable pointer
				auto memberdata = stack::stack_detail::get_as_upvalues<function_type>(L, 1);
				function_type& var = memberdata.first;
				switch (lua_gettop(L)) {
				case 1:
					return call_detail::call_wrapped<T, true, false>(L, var);
				case 2:
					return call_detail::call_wrapped<T, false, false>(L, var);
				default:
					return luaL_error(L, "sol: incorrect number of arguments to member variable function");
				}
			}

			static int call(lua_State* L) {
				return detail::static_trampoline<(&real_call)>(L);
			}

			int operator()(lua_State* L) {
				return call(L);
			}
		};
	} // function_detail
} // sol

#endif // SOL_FUNCTION_TYPES_STATELESS_HPP
