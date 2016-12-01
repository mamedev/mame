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

#ifndef SOL_PROTECTED_FUNCTION_HPP
#define SOL_PROTECTED_FUNCTION_HPP

#include "reference.hpp"
#include "stack.hpp"
#include "protected_function_result.hpp"
#include <cstdint>
#include <algorithm>

namespace sol {
	namespace detail {
		inline reference& handler_storage() {
			static sol::reference h;
			return h;
		}

		struct handler {
			const reference& target;
			int stackindex;
			handler(const reference& target) : target(target), stackindex(0) {
				if (target.valid()) {
					stackindex = lua_gettop(target.lua_state()) + 1;
					target.push();
				}
			}
			bool valid() const { return stackindex != 0; }
			~handler() {
				if (valid()) {
					lua_remove(target.lua_state(), stackindex);
				}
			}
		};
	}
	
	template <typename base_t>
	class basic_protected_function : public base_t {
	public:
		static reference& get_default_handler() {
			return detail::handler_storage();
		}

		static void set_default_handler(const reference& ref) {
			detail::handler_storage() = ref;
		}

		static void set_default_handler(reference&& ref) {
			detail::handler_storage() = std::move(ref);
		}

	private:
		call_status luacall(std::ptrdiff_t argcount, std::ptrdiff_t resultcount, detail::handler& h) const {
			return static_cast<call_status>(lua_pcallk(base_t::lua_state(), static_cast<int>(argcount), static_cast<int>(resultcount), h.stackindex, 0, nullptr));
		}

		template<std::size_t... I, typename... Ret>
		auto invoke(types<Ret...>, std::index_sequence<I...>, std::ptrdiff_t n, detail::handler& h) const {
			luacall(n, sizeof...(Ret), h);
			return stack::pop<std::tuple<Ret...>>(base_t::lua_state());
		}

		template<std::size_t I, typename Ret>
		Ret invoke(types<Ret>, std::index_sequence<I>, std::ptrdiff_t n, detail::handler& h) const {
			luacall(n, 1, h);
			return stack::pop<Ret>(base_t::lua_state());
		}

		template <std::size_t I>
		void invoke(types<void>, std::index_sequence<I>, std::ptrdiff_t n, detail::handler& h) const {
			luacall(n, 0, h);
		}

		protected_function_result invoke(types<>, std::index_sequence<>, std::ptrdiff_t n, detail::handler& h) const {
			int stacksize = lua_gettop(base_t::lua_state());
			int poststacksize = stacksize;
			int firstreturn = 1;
			int returncount = 0;
			call_status code = call_status::ok;
#ifndef SOL_NO_EXCEPTIONS
			auto onexcept = [&](const char* error) {
				h.stackindex = 0;
				if (h.target.valid()) {
					h.target.push();
					stack::push(base_t::lua_state(), error);
					lua_call(base_t::lua_state(), 1, 1);
				}
				else {
					stack::push(base_t::lua_state(), error);
				}
			};
			try {
#endif // No Exceptions
				firstreturn = (std::max)(1, static_cast<int>(stacksize - n - static_cast<int>(h.valid())));
				code = luacall(n, LUA_MULTRET, h);
				poststacksize = lua_gettop(base_t::lua_state()) - static_cast<int>(h.valid());
				returncount = poststacksize - (firstreturn - 1);
#ifndef SOL_NO_EXCEPTIONS
			}
			// Handle C++ errors thrown from C++ functions bound inside of lua
			catch (const char* error) {
				onexcept(error);
				firstreturn = lua_gettop(base_t::lua_state());
				return protected_function_result(base_t::lua_state(), firstreturn, 0, 1, call_status::runtime);
			}
			catch (const std::exception& error) {
				onexcept(error.what());
				firstreturn = lua_gettop(base_t::lua_state());
				return protected_function_result(base_t::lua_state(), firstreturn, 0, 1, call_status::runtime);
			}
			catch (...) {
				onexcept("caught (...) unknown error during protected_function call");
				firstreturn = lua_gettop(base_t::lua_state());
				return protected_function_result(base_t::lua_state(), firstreturn, 0, 1, call_status::runtime);
			}
#endif // No Exceptions
			return protected_function_result(base_t::lua_state(), firstreturn, returncount, returncount, code);
		}

	public:
		reference error_handler;

		basic_protected_function() = default;
		template <typename T, meta::enable<meta::neg<std::is_same<meta::unqualified_t<T>, basic_protected_function>>, meta::neg<std::is_same<base_t, stack_reference>>, std::is_base_of<base_t, meta::unqualified_t<T>>> = meta::enabler>
		basic_protected_function(T&& r) noexcept : base_t(std::forward<T>(r)) {
#ifdef SOL_CHECK_ARGUMENTS
			if (!is_function<meta::unqualified_t<T>>::value) {
				auto pp = stack::push_pop(*this);
				stack::check<basic_protected_function>(base_t::lua_state(), -1, type_panic);
			}
#endif // Safety
		}
		basic_protected_function(const basic_protected_function&) = default;
		basic_protected_function& operator=(const basic_protected_function&) = default;
		basic_protected_function(basic_protected_function&&) = default;
		basic_protected_function& operator=(basic_protected_function&&) = default;
		basic_protected_function(const basic_function<base_t>& b, reference eh = get_default_handler()) : base_t(b), error_handler(std::move(eh)) {}
		basic_protected_function(basic_function<base_t>&& b, reference eh = get_default_handler()) : base_t(std::move(b)), error_handler(std::move(eh)) {}
		basic_protected_function(const stack_reference& r, reference eh = get_default_handler()) : basic_protected_function(r.lua_state(), r.stack_index(), std::move(eh)) {}
		basic_protected_function(stack_reference&& r, reference eh = get_default_handler()) : basic_protected_function(r.lua_state(), r.stack_index(), std::move(eh)) {}
		template <typename Super>
		basic_protected_function(proxy_base<Super>&& p, reference eh = get_default_handler()) : basic_protected_function(p.operator basic_function<base_t>(), std::move(eh)) {}
		template <typename Super>
		basic_protected_function(const proxy_base<Super>& p, reference eh = get_default_handler()) : basic_protected_function(static_cast<basic_function<base_t>>(p), std::move(eh)) {}
		basic_protected_function(lua_State* L, int index = -1, reference eh = get_default_handler()) : base_t(L, index), error_handler(std::move(eh)) {
#ifdef SOL_CHECK_ARGUMENTS
			stack::check<basic_protected_function>(L, index, type_panic);
#endif // Safety
		}

		template<typename... Args>
		protected_function_result operator()(Args&&... args) const {
			return call<>(std::forward<Args>(args)...);
		}

		template<typename... Ret, typename... Args>
		decltype(auto) operator()(types<Ret...>, Args&&... args) const {
			return call<Ret...>(std::forward<Args>(args)...);
		}

		template<typename... Ret, typename... Args>
		decltype(auto) call(Args&&... args) const {
			detail::handler h(error_handler);
			base_t::push();
			int pushcount = stack::multi_push_reference(base_t::lua_state(), std::forward<Args>(args)...);
			return invoke(types<Ret...>(), std::make_index_sequence<sizeof...(Ret)>(), pushcount, h);
		}
	};
} // sol

#endif // SOL_FUNCTION_HPP
