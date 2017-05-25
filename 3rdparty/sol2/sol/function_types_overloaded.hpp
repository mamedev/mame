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

#ifndef SOL_FUNCTION_TYPES_OVERLOAD_HPP
#define SOL_FUNCTION_TYPES_OVERLOAD_HPP

#include "overload.hpp"
#include "call.hpp"
#include "function_types_core.hpp"

namespace sol {
	namespace function_detail {
		template <typename... Functions>
		struct overloaded_function {
			typedef std::tuple<Functions...> overload_list;
			typedef std::make_index_sequence<sizeof...(Functions)> indices;
			overload_list overloads;

			overloaded_function(overload_list set)
				: overloads(std::move(set)) {}

			overloaded_function(Functions... fxs)
				: overloads(fxs...) {

			}

			template <typename Fx, std::size_t I, typename... R, typename... Args>
			int call(types<Fx>, index_value<I>, types<R...>, types<Args...>, lua_State* L, int, int) {
				auto& func = std::get<I>(overloads);
				return call_detail::call_wrapped<void, true, false>(L, func);
			}

			int operator()(lua_State* L) {
				auto mfx = [&](auto&&... args) { return this->call(std::forward<decltype(args)>(args)...); };
				return call_detail::overload_match<Functions...>(mfx, L, 1);
			}
		};
	} // function_detail
} // sol

#endif // SOL_FUNCTION_TYPES_OVERLOAD_HPP