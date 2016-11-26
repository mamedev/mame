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

#ifndef SOL_STACK_CHECK_HPP
#define SOL_STACK_CHECK_HPP

#include "stack_core.hpp"
#include "usertype_traits.hpp"
#include "inheritance.hpp"
#include <memory>
#include <functional>
#include <utility>

namespace sol {
	namespace stack {
		namespace stack_detail {
			template <typename T, bool poptable = true>
			inline bool check_metatable(lua_State* L, int index = -2) {
				const auto& metakey = usertype_traits<T>::metatable();
				luaL_getmetatable(L, &metakey[0]);
				const type expectedmetatabletype = static_cast<type>(lua_type(L, -1));
				if (expectedmetatabletype != type::nil) {
					if (lua_rawequal(L, -1, index) == 1) {
						lua_pop(L, 1 + static_cast<int>(poptable));
						return true;
					}
				}
				lua_pop(L, 1);
				return false;
			}

			template <type expected, int(*check_func)(lua_State*, int)>
			struct basic_check {
				template <typename Handler>
				static bool check(lua_State* L, int index, Handler&& handler, record& tracking) {
					tracking.use(1);
					bool success = check_func(L, index) == 1;
					if (!success) {
						// expected type, actual type
						handler(L, index, expected, type_of(L, index));
					}
					return success;
				}
			};
		} // stack_detail

		template <typename T, type expected, typename>
		struct checker {
			template <typename Handler>
			static bool check(lua_State* L, int index, Handler&& handler, record& tracking) {
				tracking.use(1);
				const type indextype = type_of(L, index);
				bool success = expected == indextype;
				if (!success) {
					// expected type, actual type
					handler(L, index, expected, indextype);
				}
				return success;
			}
		};

		template<typename T>
		struct checker<T, type::number, std::enable_if_t<std::is_integral<T>::value>> {
			template <typename Handler>
			static bool check(lua_State* L, int index, Handler&& handler, record& tracking) {
				tracking.use(1);
				bool success = lua_isinteger(L, index) == 1;
				if (!success) {
					// expected type, actual type
					handler(L, index, type::number, type_of(L, index));
				}
				return success;
			}
		};

		template<typename T>
		struct checker<T, type::number, std::enable_if_t<std::is_floating_point<T>::value>> {
			template <typename Handler>
			static bool check(lua_State* L, int index, Handler&& handler, record& tracking) {
				tracking.use(1);
				bool success = lua_isnumber(L, index) == 1;
				if (!success) {
					// expected type, actual type
					handler(L, index, type::number, type_of(L, index));
				}
				return success;
			}
		};

		template <type expected, typename C>
		struct checker<nil_t, expected, C> {
			template <typename Handler>
			static bool check(lua_State* L, int index, Handler&& handler, record& tracking) {
				bool success = lua_isnil(L, index);
				if (success) {
					tracking.use(1);
					return success;
				}
				tracking.use(0);
				success = lua_isnone(L, index);
				if (!success) {
					// expected type, actual type
					handler(L, index, expected, type_of(L, index));
				}
				return success;
			}
		};

		template <type expected, typename C>
		struct checker<nullopt_t, expected, C> : checker<nil_t> {};

		template <typename C>
		struct checker<this_state, type::poly, C> {
			template <typename Handler>
			static bool check(lua_State*, int, Handler&&, record& tracking) {
				tracking.use(0);
				return true;
			}
		};

		template <typename C>
		struct checker<variadic_args, type::poly, C> {
			template <typename Handler>
			static bool check(lua_State*, int, Handler&&, record& tracking) {
				tracking.use(0);
				return true;
			}
		};

		template <typename C>
		struct checker<type, type::poly, C> {
			template <typename Handler>
			static bool check(lua_State*, int, Handler&&, record& tracking) {
				tracking.use(0);
				return true;
			}
		};

		template <typename T, typename C>
		struct checker<T, type::poly, C> {
			template <typename Handler>
			static bool check(lua_State* L, int index, Handler&& handler, record& tracking) {
				tracking.use(1);
				bool success = !lua_isnone(L, index);
				if (!success) {
					// expected type, actual type
					handler(L, index, type::none, type_of(L, index));
				}
				return success;
			}
		};

		template <typename T, typename C>
		struct checker<T, type::lightuserdata, C> {
			template <typename Handler>
			static bool check(lua_State* L, int index, Handler&& handler, record& tracking) {
				tracking.use(1);
				type t = type_of(L, index);
				bool success = t == type::userdata || t == type::lightuserdata;
				if (!success) {
					// expected type, actual type
					handler(L, index, type::lightuserdata, t);
				}
				return success;
			}
		};

		template <typename C>
		struct checker<userdata_value, type::userdata, C> {
			template <typename Handler>
			static bool check(lua_State* L, int index, Handler&& handler, record& tracking) {
				tracking.use(1);
				type t = type_of(L, index);
				bool success = t == type::userdata;
				if (!success) {
					// expected type, actual type
					handler(L, index, type::userdata, t);
				}
				return success;
			}
		};

		template <typename T, typename C>
		struct checker<user<T>, type::userdata, C> : checker<user<T>, type::lightuserdata, C> {};

		template <typename T, typename C>
		struct checker<non_null<T>, type::userdata, C> : checker<T, lua_type_of<T>::value, C> {};

		template <typename C>
		struct checker<lua_CFunction, type::function, C> : stack_detail::basic_check<type::function, lua_iscfunction> {};
		template <typename C>
		struct checker<std::remove_pointer_t<lua_CFunction>, type::function, C> : checker<lua_CFunction, type::function, C> {};
		template <typename C>
		struct checker<c_closure, type::function, C> : checker<lua_CFunction, type::function, C> {};

		template <typename T, typename C>
		struct checker<T, type::function, C> {
			template <typename Handler>
			static bool check(lua_State* L, int index, Handler&& handler, record& tracking) {
				tracking.use(1);
				type t = type_of(L, index);
				if (t == type::nil || t == type::none || t == type::function) {
					// allow for nil to be returned
					return true;
				}
				if (t != type::userdata && t != type::table) {
					handler(L, index, type::function, t);
					return false;
				}
				// Do advanced check for call-style userdata?
				static const auto& callkey = name_of(meta_function::call);
				if (lua_getmetatable(L, index) == 0) {
					// No metatable, no __call key possible
					handler(L, index, type::function, t);
					return false;
				}
				if (lua_isnoneornil(L, -1)) {
					lua_pop(L, 1);
					handler(L, index, type::function, t);
					return false;
				}
				lua_getfield(L, -1, &callkey[0]);
				if (lua_isnoneornil(L, -1)) {
					lua_pop(L, 2);
					handler(L, index, type::function, t);
					return false;
				}
				// has call, is definitely a function
				lua_pop(L, 2);
				return true;
			}
		};

		template <typename T, typename C>
		struct checker<T, type::table, C> {
			template <typename Handler>
			static bool check(lua_State* L, int index, Handler&& handler, record& tracking) {
				tracking.use(1);
				type t = type_of(L, index);
				if (t == type::table) {
					return true;
				}
				if (t != type::userdata) {
					handler(L, index, type::function, t);
					return false;
				}
				return true;
			}
		};

		template <typename T, typename C>
		struct checker<detail::as_value_tag<T>, type::userdata, C> {
			template <typename U, typename Handler>
			static bool check(types<U>, lua_State* L, type indextype, int index, Handler&& handler, record& tracking) {
				tracking.use(1);
				if (indextype != type::userdata) {
					handler(L, index, type::userdata, indextype);
					return false;
				}
				if (meta::any<std::is_same<T, lightuserdata_value>, std::is_same<T, userdata_value>, std::is_same<T, userdata>, std::is_same<T, lightuserdata>>::value)
					return true;
				if (lua_getmetatable(L, index) == 0) {
					return true;
				}
				int metatableindex = lua_gettop(L);
				if (stack_detail::check_metatable<U>(L, metatableindex))
					return true;
				if (stack_detail::check_metatable<U*>(L, metatableindex))
					return true;
				if (stack_detail::check_metatable<detail::unique_usertype<U>>(L, metatableindex))
					return true;
				bool success = false;
				if (detail::has_derived<T>::value) {
					auto pn = stack::pop_n(L, 1);
					lua_pushstring(L, &detail::base_class_check_key()[0]);
					lua_rawget(L, metatableindex);
					if (type_of(L, -1) != type::nil) {
						void* basecastdata = lua_touserdata(L, -1);
						detail::inheritance_check_function ic = (detail::inheritance_check_function)basecastdata;
						success = ic(detail::id_for<T>::value);
					}
				}
				if (!success) {
					lua_pop(L, 1);
					handler(L, index, type::userdata, indextype);
					return false;
				}
				lua_pop(L, 1);
				return true;
			}
		};

		template <typename T, typename C>
		struct checker<T, type::userdata, C> {
			template <typename Handler>
			static bool check(lua_State* L, int index, Handler&& handler, record& tracking) {
				const type indextype = type_of(L, index);
				return checker<detail::as_value_tag<T>, type::userdata, C>{}.check(types<T>(), L, indextype, index, std::forward<Handler>(handler), tracking);
			}
		};

		template <typename T, typename C>
		struct checker<T*, type::userdata, C> {
			template <typename Handler>
			static bool check(lua_State* L, int index, Handler&& handler, record& tracking) {
				const type indextype = type_of(L, index);
				// Allow nil to be transformed to nullptr
				if (indextype == type::nil) {
					tracking.use(1);
					return true;
				}
				return checker<meta::unqualified_t<T>, type::userdata, C>{}.check(L, index, std::forward<Handler>(handler), tracking);
			}
		};

		template<typename T>
		struct checker<T, type::userdata, std::enable_if_t<is_unique_usertype<T>::value>> {
			template <typename Handler>
			static bool check(lua_State* L, int index, Handler&& handler, record& tracking) {
				return checker<typename unique_usertype_traits<T>::type, type::userdata>{}.check(L, index, std::forward<Handler>(handler), tracking);
			}
		};

		template<typename T, typename C>
		struct checker<std::reference_wrapper<T>, type::userdata, C> {
			template <typename Handler>
			static bool check(lua_State* L, int index, Handler&& handler, record& tracking) {
				return checker<T, type::userdata, C>{}.check(L, index, std::forward<Handler>(handler), tracking);
			}
		};

		template<typename... Args, typename C>
		struct checker<std::tuple<Args...>, type::poly, C> {
			template <typename Handler>
			static bool check(lua_State* L, int index, Handler&& handler, record& tracking) {
				return stack::multi_check<Args...>(L, index, std::forward<Handler>(handler), tracking);
			}
		};

		template<typename A, typename B, typename C>
		struct checker<std::pair<A, B>, type::poly, C> {
			template <typename Handler>
			static bool check(lua_State* L, int index, Handler&& handler, record& tracking) {
				return stack::multi_check<A, B>(L, index, std::forward<Handler>(handler), tracking);
			}
		};

		template<typename T, typename C>
		struct checker<optional<T>, type::poly, C> {
			template <typename Handler>
			static bool check(lua_State* L, int index, Handler&&, record& tracking) {
				type t = type_of(L, index);
				if (t == type::none) {
					tracking.use(0);
					return true;
				}
				if (t == type::nil) {
					tracking.use(1);
					return true;
				}
				return stack::check<T>(L, index, no_panic, tracking);
			}
		};
	} // stack
} // sol

#endif // SOL_STACK_CHECK_HPP
