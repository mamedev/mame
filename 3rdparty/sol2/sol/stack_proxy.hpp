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

#ifndef SOL_STACK_PROXY_HPP
#define SOL_STACK_PROXY_HPP

#include "stack.hpp"
#include "function.hpp"
#include "protected_function.hpp"
#include "proxy_base.hpp"

namespace sol {
	struct stack_proxy : public proxy_base<stack_proxy> {
	private:
		lua_State* L;
		int index;

	public:
		stack_proxy() : L(nullptr), index(0) {}
		stack_proxy(lua_State* L, int index) : L(L), index(index) {}

		template<typename T>
		decltype(auto) get() const {
			return stack::get<T>(L, stack_index());
		}

		int push() const {
			lua_pushvalue(L, index);
			return 1;
		}

		lua_State* lua_state() const { return L; }
		int stack_index() const { return index; }

		template<typename... Ret, typename... Args>
		decltype(auto) call(Args&&... args) {
			return get<function>().template call<Ret...>(std::forward<Args>(args)...);
		}

		template<typename... Args>
		decltype(auto) operator()(Args&&... args) {
			return call<>(std::forward<Args>(args)...);
		}
	};

	namespace stack {
		template <>
		struct getter<stack_proxy> {
			static stack_proxy get(lua_State* L, int index = -1) {
				return stack_proxy(L, index);
			}
		};

		template <>
		struct pusher<stack_proxy> {
			static int push(lua_State*, const stack_proxy& ref) {
				return ref.push();
			}
		};
	} // stack

	namespace detail {
		template <>
		struct is_speshul<function_result> : std::true_type {};
		template <>
		struct is_speshul<protected_function_result> : std::true_type {};

		template <std::size_t I, typename... Args, typename T>
		stack_proxy get(types<Args...>, index_value<0>, index_value<I>, const T& fr) {
			return stack_proxy(fr.lua_state(), static_cast<int>(fr.stack_index() + I));
		}

		template <std::size_t I, std::size_t N, typename Arg, typename... Args, typename T, meta::enable<meta::boolean<(N > 0)>> = meta::enabler>
		stack_proxy get(types<Arg, Args...>, index_value<N>, index_value<I>, const T& fr) {
			return get(types<Args...>(), index_value<N - 1>(), index_value<I + lua_size<Arg>::value>(), fr);
		}
	}

	template <>
	struct tie_size<function_result> : std::integral_constant<std::size_t, SIZE_MAX> {};

	template <std::size_t I>
	stack_proxy get(const function_result& fr) {
		return stack_proxy(fr.lua_state(), static_cast<int>(fr.stack_index() + I));
	}

	template <std::size_t I, typename... Args>
	stack_proxy get(types<Args...> t, const function_result& fr) {
		return detail::get(t, index_value<I>(), index_value<0>(), fr);
	}

	template <>
	struct tie_size<protected_function_result> : std::integral_constant<std::size_t, SIZE_MAX> {};

	template <std::size_t I>
	stack_proxy get(const protected_function_result& fr) {
		return stack_proxy(fr.lua_state(), static_cast<int>(fr.stack_index() + I));
	}

	template <std::size_t I, typename... Args>
	stack_proxy get(types<Args...> t, const protected_function_result& fr) {
		return detail::get(t, index_value<I>(), index_value<0>(), fr);
	}
} // sol

#endif // SOL_STACK_PROXY_HPP
