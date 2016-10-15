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

#ifndef SOL_STACK_REFERENCE_HPP
#define SOL_STACK_REFERENCE_HPP

namespace sol {
	class stack_reference {
	private:
		lua_State* L = nullptr;
		int index = 0;

	protected:
		int registry_index() const noexcept {
			return LUA_NOREF;
		}

	public:
		stack_reference() noexcept = default;
		stack_reference(nil_t) noexcept : stack_reference() {};
		stack_reference(lua_State* L, int i) noexcept : L(L), index(lua_absindex(L, i)) {}
		stack_reference(lua_State* L, absolute_index i) noexcept : L(L), index(i) {}
		stack_reference(lua_State* L, raw_index i) noexcept : L(L), index(i) {}
		stack_reference(stack_reference&& o) noexcept = default;
		stack_reference& operator=(stack_reference&&) noexcept = default;
		stack_reference(const stack_reference&) noexcept = default;
		stack_reference& operator=(const stack_reference&) noexcept = default;

		int push() const noexcept {
			lua_pushvalue(L, index);
			return 1;
		}

		void pop(int n = 1) const noexcept {
			lua_pop(lua_state(), n);
		}

		int stack_index() const noexcept {
			return index;
		}

		type get_type() const noexcept {
			int result = lua_type(L, index);
			return static_cast<type>(result);
		}

		lua_State* lua_state() const noexcept {
			return L;
		}

		bool valid() const noexcept {
			type t = get_type();
			return t != type::nil && t != type::none;
		}
	};

	inline bool operator== (const stack_reference& l, const stack_reference& r) {
		return lua_compare(l.lua_state(), l.stack_index(), r.stack_index(), LUA_OPEQ) == 0;
	}

	inline bool operator!= (const stack_reference& l, const stack_reference& r) {
		return !operator==(l, r);
	}
} // sol

#endif // SOL_STACK_REFERENCE_HPP
