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

#ifndef SOL_STACK_CHECK_GET_HPP
#define SOL_STACK_CHECK_GET_HPP

#include "stack_core.hpp"
#include "stack_get.hpp"
#include "stack_check.hpp"
#include "optional.hpp"

namespace sol {
	namespace stack {
		template <typename T, typename>
		struct check_getter {
			typedef decltype(stack_detail::unchecked_get<T>(nullptr, 0, std::declval<record&>())) R;

			template <typename Handler>
			static optional<R> get(lua_State* L, int index, Handler&& handler, record& tracking) {
				if (!check<T>(L, index, std::forward<Handler>(handler))) {
					tracking.use(static_cast<int>(!lua_isnone(L, index)));
					return nullopt;
				}
				return stack_detail::unchecked_get<T>(L, index, tracking);
			}
		};

		template <typename T>
		struct check_getter<optional<T>> {
			template <typename Handler>
			static decltype(auto) get(lua_State* L, int index, Handler&&, record& tracking) {
				return check_get<T>(L, index, no_panic, tracking);
			}
		};

		template <typename T>
		struct check_getter<T, std::enable_if_t<std::is_integral<T>::value && lua_type_of<T>::value == type::number>> {
			template <typename Handler>
			static optional<T> get(lua_State* L, int index, Handler&& handler, record& tracking) {
				int isnum = 0;
				lua_Integer value = lua_tointegerx(L, index, &isnum);
				if (isnum == 0) {
					type t = type_of(L, index);
					tracking.use(static_cast<int>(t != type::none));
					handler(L, index, type::number, t);
					return nullopt;
				}
				tracking.use(1);
				return static_cast<T>(value);
			}
		};

		template <typename T>
		struct check_getter<T, std::enable_if_t<std::is_enum<T>::value && !meta::any_same<T, meta_function, type>::value>> {
			template <typename Handler>
			static optional<T> get(lua_State* L, int index, Handler&& handler, record& tracking) {
				int isnum = 0;
				lua_Integer value = lua_tointegerx(L, index, &isnum);
				if (isnum == 0) {
					type t = type_of(L, index);
					tracking.use(static_cast<int>(t != type::none));
					handler(L, index, type::number, t);
					return nullopt;
				}
				tracking.use(1);
				return static_cast<T>(value);
			}
		};

		template <typename T>
		struct check_getter<T, std::enable_if_t<std::is_floating_point<T>::value>> {
			template <typename Handler>
			static optional<T> get(lua_State* L, int index, Handler&& handler, record& tracking) {
				int isnum = 0;
				lua_Number value = lua_tonumberx(L, index, &isnum);
				if (isnum == 0) {
					type t = type_of(L, index);
					tracking.use(static_cast<int>(t != type::none));
					handler(L, index, type::number, t);
					return nullopt;
				}
				tracking.use(1);
				return static_cast<T>(value);
			}
		};

		template <typename T>
		struct getter<optional<T>> {
			static decltype(auto) get(lua_State* L, int index, record& tracking) {
				return check_get<T>(L, index, no_panic, tracking);
			}
		};
	} // stack
} // sol

#endif // SOL_STACK_CHECK_GET_HPP
