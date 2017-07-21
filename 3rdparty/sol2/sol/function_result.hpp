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

#ifndef SOL_FUNCTION_RESULT_HPP
#define SOL_FUNCTION_RESULT_HPP

#include "reference.hpp"
#include "tuple.hpp"
#include "stack.hpp"
#include "proxy_base.hpp"
#include <cstdint>

namespace sol {
	struct function_result : public proxy_base<function_result> {
	private:
		lua_State* L;
		int index;
		int returncount;

	public:
		function_result() = default;
		function_result(lua_State* Ls, int idx = -1, int retnum = 0) : L(Ls), index(idx), returncount(retnum) {

		}
		function_result(const function_result&) = default;
		function_result& operator=(const function_result&) = default;
		function_result(function_result&& o) : L(o.L), index(o.index), returncount(o.returncount) {
			// Must be manual, otherwise destructor will screw us
			// return count being 0 is enough to keep things clean
			// but will be thorough
			o.L = nullptr;
			o.index = 0;
			o.returncount = 0;
		}
		function_result& operator=(function_result&& o) {
			L = o.L;
			index = o.index;
			returncount = o.returncount;
			// Must be manual, otherwise destructor will screw us
			// return count being 0 is enough to keep things clean
			// but will be thorough
			o.L = nullptr;
			o.index = 0;
			o.returncount = 0;
			return *this;
		}

		template<typename T>
		decltype(auto) get() const {
			return stack::get<T>(L, index);
		}

		call_status status() const noexcept {
			return call_status::ok;
		}

		bool valid() const noexcept {
			return status() == call_status::ok || status() == call_status::yielded;
		}

		lua_State* lua_state() const { return L; };
		int stack_index() const { return index; };

		~function_result() {
			lua_pop(L, returncount);
		}
	};
} // sol

#endif // SOL_FUNCTION_RESULT_HPP
