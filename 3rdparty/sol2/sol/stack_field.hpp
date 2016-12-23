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

#ifndef SOL_STACK_FIELD_HPP
#define SOL_STACK_FIELD_HPP

#include "stack_core.hpp"
#include "stack_push.hpp"
#include "stack_get.hpp"
#include "stack_check_get.hpp"

namespace sol {
	namespace stack {
		template <typename T, bool, bool, typename>
		struct field_getter {
			template <typename Key>
			void get(lua_State* L, Key&& key, int tableindex = -2) {
				push(L, std::forward<Key>(key));
				lua_gettable(L, tableindex);
			}
		};

		template <typename T, bool global, typename C>
		struct field_getter<T, global, true, C> {
			template <typename Key>
			void get(lua_State* L, Key&& key, int tableindex = -2) {
				push(L, std::forward<Key>(key));
				lua_rawget(L, tableindex);
			}
		};

		template <bool b, bool raw, typename C>
		struct field_getter<metatable_key_t, b, raw, C> {
			void get(lua_State* L, metatable_key_t, int tableindex = -1) {
				if (lua_getmetatable(L, tableindex) == 0)
					push(L, nil);
			}
		};

		template <typename T, bool raw>
		struct field_getter<T, true, raw, std::enable_if_t<meta::is_c_str<T>::value>> {
			template <typename Key>
			void get(lua_State* L, Key&& key, int = -1) {
				lua_getglobal(L, &key[0]);
			}
		};

		template <typename T>
		struct field_getter<T, false, false, std::enable_if_t<meta::is_c_str<T>::value>> {
			template <typename Key>
			void get(lua_State* L, Key&& key, int tableindex = -1) {
				lua_getfield(L, tableindex, &key[0]);
			}
		};

#if SOL_LUA_VERSION >= 503
		template <typename T>
		struct field_getter<T, false, false, std::enable_if_t<std::is_integral<T>::value>> {
			template <typename Key>
			void get(lua_State* L, Key&& key, int tableindex = -1) {
				lua_geti(L, tableindex, static_cast<lua_Integer>(key));
			}
		};
#endif // Lua 5.3.x

#if SOL_LUA_VERSION >= 502
		template <typename C>
		struct field_getter<void*, false, true, C> {
			void get(lua_State* L, void* key, int tableindex = -1) {
				lua_rawgetp(L, tableindex, key);
			}
		};
#endif // Lua 5.3.x

		template <typename T>
		struct field_getter<T, false, true, std::enable_if_t<std::is_integral<T>::value>> {
			template <typename Key>
			void get(lua_State* L, Key&& key, int tableindex = -1) {
				lua_rawgeti(L, tableindex, static_cast<lua_Integer>(key));
			}
		};

		template <typename... Args, bool b, bool raw, typename C>
		struct field_getter<std::tuple<Args...>, b, raw, C> {
			template <std::size_t... I, typename Keys>
			void apply(std::index_sequence<0, I...>, lua_State* L, Keys&& keys, int tableindex) {
				get_field<b, raw>(L, detail::forward_get<0>(keys), tableindex);
				void(detail::swallow{ (get_field<false, raw>(L, detail::forward_get<I>(keys)), 0)... });
				reference saved(L, -1);
				lua_pop(L, static_cast<int>(sizeof...(I)));
				saved.push();
			}

			template <typename Keys>
			void get(lua_State* L, Keys&& keys) {
				apply(std::make_index_sequence<sizeof...(Args)>(), L, std::forward<Keys>(keys), lua_absindex(L, -1));
			}

			template <typename Keys>
			void get(lua_State* L, Keys&& keys, int tableindex) {
				apply(std::make_index_sequence<sizeof...(Args)>(), L, std::forward<Keys>(keys), tableindex);
			}
		};

		template <typename A, typename B, bool b, bool raw, typename C>
		struct field_getter<std::pair<A, B>, b, raw, C> {
			template <typename Keys>
			void get(lua_State* L, Keys&& keys, int tableindex) {
				get_field<b, raw>(L, detail::forward_get<0>(keys), tableindex);
				get_field<false, raw>(L, detail::forward_get<1>(keys));
				reference saved(L, -1);
				lua_pop(L, static_cast<int>(2));
				saved.push();
			}

			template <typename Keys>
			void get(lua_State* L, Keys&& keys) {
				get_field<b, raw>(L, detail::forward_get<0>(keys));
				get_field<false, raw>(L, detail::forward_get<1>(keys));
				reference saved(L, -1);
				lua_pop(L, static_cast<int>(2));
				saved.push();
			}
		};

		template <typename T, bool, bool, typename>
		struct field_setter {
			template <typename Key, typename Value>
			void set(lua_State* L, Key&& key, Value&& value, int tableindex = -3) {
				push(L, std::forward<Key>(key));
				push(L, std::forward<Value>(value));
				lua_settable(L, tableindex);
			}
		};

		template <typename T, bool b, typename C>
		struct field_setter<T, b, true, C> {
			template <typename Key, typename Value>
			void set(lua_State* L, Key&& key, Value&& value, int tableindex = -3) {
				push(L, std::forward<Key>(key));
				push(L, std::forward<Value>(value));
				lua_rawset(L, tableindex);
			}
		};

		template <bool b, bool raw, typename C>
		struct field_setter<metatable_key_t, b, raw, C> {
			template <typename Value>
			void set(lua_State* L, metatable_key_t, Value&& value, int tableindex = -2) {
				push(L, std::forward<Value>(value));
				lua_setmetatable(L, tableindex);
			}
		};

		template <typename T, bool raw>
		struct field_setter<T, true, raw, std::enable_if_t<meta::is_c_str<T>::value>> {
			template <typename Key, typename Value>
			void set(lua_State* L, Key&& key, Value&& value, int = -2) {
				push(L, std::forward<Value>(value));
				lua_setglobal(L, &key[0]);
			}
		};

		template <typename T>
		struct field_setter<T, false, false, std::enable_if_t<meta::is_c_str<T>::value>> {
			template <typename Key, typename Value>
			void set(lua_State* L, Key&& key, Value&& value, int tableindex = -2) {
				push(L, std::forward<Value>(value));
				lua_setfield(L, tableindex, &key[0]);
			}
		};

#if SOL_LUA_VERSION >= 503
		template <typename T>
		struct field_setter<T, false, false, std::enable_if_t<std::is_integral<T>::value>> {
			template <typename Key, typename Value>
			void set(lua_State* L, Key&& key, Value&& value, int tableindex = -2) {
				push(L, std::forward<Value>(value));
				lua_seti(L, tableindex, static_cast<lua_Integer>(key));
			}
		};
#endif // Lua 5.3.x

		template <typename T>
		struct field_setter<T, false, true, std::enable_if_t<std::is_integral<T>::value>> {
			template <typename Key, typename Value>
			void set(lua_State* L, Key&& key, Value&& value, int tableindex = -2) {
				push(L, std::forward<Value>(value));
				lua_rawseti(L, tableindex, static_cast<lua_Integer>(key));
			}
		};

#if SOL_LUA_VERSION >= 502
		template <typename C>
		struct field_setter<void*, false, true, C> {
			template <typename Key, typename Value>
			void set(lua_State* L, void* key, Value&& value, int tableindex = -2) {
				push(L, std::forward<Value>(value));
				lua_rawsetp(L, tableindex, key);
			}
		};
#endif // Lua 5.2.x

		template <typename... Args, bool b, bool raw, typename C>
		struct field_setter<std::tuple<Args...>, b, raw, C> {
			template <bool g, std::size_t I, typename Key, typename Value>
			void apply(std::index_sequence<I>, lua_State* L, Key&& keys, Value&& value, int tableindex) {
				I < 1 ?
					set_field<g, raw>(L, detail::forward_get<I>(keys), std::forward<Value>(value), tableindex) :
					set_field<g, raw>(L, detail::forward_get<I>(keys), std::forward<Value>(value));
			}

			template <bool g, std::size_t I0, std::size_t I1, std::size_t... I, typename Keys, typename Value>
			void apply(std::index_sequence<I0, I1, I...>, lua_State* L, Keys&& keys, Value&& value, int tableindex) {
				I0 < 1 ? get_field<g, raw>(L, detail::forward_get<I0>(keys), tableindex) : get_field<g, raw>(L, detail::forward_get<I0>(keys), -1);
				apply<false>(std::index_sequence<I1, I...>(), L, std::forward<Keys>(keys), std::forward<Value>(value), -1);
			}

			template <bool g, std::size_t I0, std::size_t... I, typename Keys, typename Value>
			void top_apply(std::index_sequence<I0, I...>, lua_State* L, Keys&& keys, Value&& value, int tableindex) {
				apply<g>(std::index_sequence<I0, I...>(), L, std::forward<Keys>(keys), std::forward<Value>(value), tableindex);
				lua_pop(L, static_cast<int>(sizeof...(I)));
			}

			template <typename Keys, typename Value>
			void set(lua_State* L, Keys&& keys, Value&& value, int tableindex = -3) {
				top_apply<b>(std::make_index_sequence<sizeof...(Args)>(), L, std::forward<Keys>(keys), std::forward<Value>(value), tableindex);
			}
		};

		template <typename A, typename B, bool b, bool raw, typename C>
		struct field_setter<std::pair<A, B>, b, raw, C> {
			template <typename Keys, typename Value>
			void set(lua_State* L, Keys&& keys, Value&& value, int tableindex = -1) {
				get_field<b, raw>(L, detail::forward_get<0>(keys), tableindex);
				set_field<false, raw>(L, detail::forward_get<1>(keys), std::forward<Value>(value));
				lua_pop(L, 1);
			}
		};
	} // stack
} // sol

#endif // SOL_STACK_FIELD_HPP
