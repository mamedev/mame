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

#ifndef SOL_STACK_GUARD_HPP
#define SOL_STACK_GUARD_HPP

#include "compatibility/version.hpp"
#include "error.hpp"
#include <functional>

namespace sol {
	namespace detail {
		inline void stack_fail(int, int) {
#ifndef SOL_NO_EXCEPTIONS
			throw error(detail::direct_error, "imbalanced stack after operation finish");
#else
			// Lol, what do you want, an error printout? :3c
			// There's no sane default here. The right way would be C-style abort(), and that's not acceptable, so
			// hopefully someone will register their own stack_fail thing for the `fx` parameter of stack_guard.
#endif // No Exceptions
		}
	} // detail

	struct stack_guard {
		lua_State* L;
		int top;
		std::function<void(int, int)> on_mismatch;

		stack_guard(lua_State* L) : stack_guard(L, lua_gettop(L)) {}
		stack_guard(lua_State* L, int top, std::function<void(int, int)> fx = detail::stack_fail) : L(L), top(top), on_mismatch(std::move(fx)) {}
		bool check_stack(int modification = 0) const {
			int bottom = lua_gettop(L) + modification;
			if (top == bottom) {
				return true;
			}
			on_mismatch(top, bottom);
			return false;
		}
		~stack_guard() {
			check_stack();
		}
	};
} // sol

#endif // SOL_STACK_GUARD_HPP
