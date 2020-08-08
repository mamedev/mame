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

#ifndef SOL_OBJECT_HPP
#define SOL_OBJECT_HPP

#include "reference.hpp"
#include "stack.hpp"
#include "userdata.hpp"
#include "as_args.hpp"
#include "variadic_args.hpp"
#include "optional.hpp"

namespace sol {

	template <typename R = reference, bool should_pop = !std::is_base_of<stack_reference, R>::value, typename T>
	R make_reference(lua_State* L, T&& value) {
		int backpedal = stack::push(L, std::forward<T>(value));
		R r = stack::get<R>(L, -backpedal);
		if (should_pop) {
			lua_pop(L, backpedal);
		}
		return r;
	}

	template <typename T, typename R = reference, bool should_pop = !std::is_base_of<stack_reference, R>::value, typename... Args>
	R make_reference(lua_State* L, Args&&... args) {
		int backpedal = stack::push_specific<T>(L, std::forward<Args>(args)...);
		R r = stack::get<R>(L, -backpedal);
		if (should_pop) {
			lua_pop(L, backpedal);
		}
		return r;
	}

	template <typename base_t>
	class basic_object : public base_t {
	private:
		template<typename T>
		decltype(auto) as_stack(std::true_type) const {
			return stack::get<T>(base_t::lua_state(), base_t::stack_index());
		}

		template<typename T>
		decltype(auto) as_stack(std::false_type) const {
			base_t::push();
			return stack::pop<T>(base_t::lua_state());
		}

		template<typename T>
		bool is_stack(std::true_type) const {
			return stack::check<T>(base_t::lua_state(), base_t::stack_index(), no_panic);
		}

		template<typename T>
		bool is_stack(std::false_type) const {
			auto pp = stack::push_pop(*this);
			return stack::check<T>(base_t::lua_state(), -1, no_panic);
		}

		template <bool invert_and_pop = false>
		basic_object(std::integral_constant<bool, invert_and_pop>, lua_State* L, int index = -1) noexcept : base_t(L, index) {
			if (invert_and_pop) {
				lua_pop(L, -index);
			}
		}

	public:
		basic_object() noexcept = default;
		template <typename T, meta::enable<meta::neg<std::is_same<meta::unqualified_t<T>, basic_object>>, meta::neg<std::is_same<base_t, stack_reference>>, std::is_base_of<base_t, meta::unqualified_t<T>>> = meta::enabler>
		basic_object(T&& r) : base_t(std::forward<T>(r)) {}
		basic_object(lua_nil_t r) : base_t(r) {}
		basic_object(const basic_object&) = default;
		basic_object(basic_object&&) = default;
		basic_object(const stack_reference& r) noexcept : basic_object(r.lua_state(), r.stack_index()) {}
		basic_object(stack_reference&& r) noexcept : basic_object(r.lua_state(), r.stack_index()) {}
		template <typename Super>
		basic_object(const proxy_base<Super>& r) noexcept : basic_object(r.operator basic_object()) {}
		template <typename Super>
		basic_object(proxy_base<Super>&& r) noexcept : basic_object(r.operator basic_object()) {}
		basic_object(lua_State* L, int index = -1) noexcept : base_t(L, index) {}
		basic_object(lua_State* L, ref_index index) noexcept : base_t(L, index) {}
		template <typename T, typename... Args>
		basic_object(lua_State* L, in_place_type_t<T>, Args&&... args) noexcept : basic_object(std::integral_constant<bool, !std::is_base_of<stack_reference, base_t>::value>(), L, -stack::push_specific<T>(L, std::forward<Args>(args)...)) {}
		template <typename T, typename... Args>
		basic_object(lua_State* L, in_place_t, T&& arg, Args&&... args) noexcept : basic_object(L, in_place<T>, std::forward<T>(arg), std::forward<Args>(args)...) {}
		basic_object& operator=(const basic_object&) = default;
		basic_object& operator=(basic_object&&) = default;
		basic_object& operator=(const base_t& b) { base_t::operator=(b); return *this; }
		basic_object& operator=(base_t&& b) { base_t::operator=(std::move(b)); return *this; }
		template <typename Super>
		basic_object& operator=(const proxy_base<Super>& r) { this->operator=(r.operator basic_object()); return *this; }
		template <typename Super>
		basic_object& operator=(proxy_base<Super>&& r) { this->operator=(r.operator basic_object()); return *this; }

		template<typename T>
		decltype(auto) as() const {
			return as_stack<T>(std::is_same<base_t, stack_reference>());
		}

		template<typename T>
		bool is() const {
			if (!base_t::valid())
				return false;
			return is_stack<T>(std::is_same<base_t, stack_reference>());
		}
	};

	template <typename T>
	object make_object(lua_State* L, T&& value) {
		return make_reference<object, true>(L, std::forward<T>(value));
	}

	template <typename T, typename... Args>
	object make_object(lua_State* L, Args&&... args) {
		return make_reference<T, object, true>(L, std::forward<Args>(args)...);
	}

	inline bool operator==(const object& lhs, const lua_nil_t&) {
		return !lhs.valid();
	}

	inline bool operator==(const lua_nil_t&, const object& rhs) {
		return !rhs.valid();
	}

	inline bool operator!=(const object& lhs, const lua_nil_t&) {
		return lhs.valid();
	}

	inline bool operator!=(const lua_nil_t&, const object& rhs) {
		return rhs.valid();
	}
} // sol

#endif // SOL_OBJECT_HPP
