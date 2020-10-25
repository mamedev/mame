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

#ifndef SOL_STACK_PUSH_HPP
#define SOL_STACK_PUSH_HPP

#include "stack_core.hpp"
#include "raii.hpp"
#include "optional.hpp"
#include <memory>
#ifdef SOL_CODECVT_SUPPORT
#include <codecvt>
#include <locale>
#endif

namespace sol {
	namespace stack {
		template <typename T>
		struct pusher<detail::as_value_tag<T>> {
			template <typename F, typename... Args>
			static int push_fx(lua_State* L, F&& f, Args&&... args) {
				// Basically, we store all user-data like this:
				// If it's a movable/copyable value (no std::ref(x)), then we store the pointer to the new
				// data in the first sizeof(T*) bytes, and then however many bytes it takes to
				// do the actual object. Things that are std::ref or plain T* are stored as 
				// just the sizeof(T*), and nothing else.
				T** pointerpointer = static_cast<T**>(lua_newuserdata(L, sizeof(T*) + sizeof(T)));
				T*& referencereference = *pointerpointer;
				T* allocationtarget = reinterpret_cast<T*>(pointerpointer + 1);
				referencereference = allocationtarget;
				std::allocator<T> alloc{};
				alloc.construct(allocationtarget, std::forward<Args>(args)...);
				f();
				return 1;
			}

			template <typename K, typename... Args>
			static int push_keyed(lua_State* L, K&& k, Args&&... args) {
				return push_fx(L, [&L, &k]() {
					luaL_newmetatable(L, &k[0]);
					lua_setmetatable(L, -2);
				}, std::forward<Args>(args)...);
			}

			template <typename... Args>
			static int push(lua_State* L, Args&&... args) {
				return push_keyed(L, usertype_traits<T>::metatable(), std::forward<Args>(args)...);
			}
		};

		template <typename T>
		struct pusher<detail::as_pointer_tag<T>> {
			template <typename F>
			static int push_fx(lua_State* L, F&& f, T* obj) {
				if (obj == nullptr)
					return stack::push(L, lua_nil);
				T** pref = static_cast<T**>(lua_newuserdata(L, sizeof(T*)));
				*pref = obj;
				f();
				return 1;
			}

			template <typename K>
			static int push_keyed(lua_State* L, K&& k, T* obj) {
				return push_fx(L, [&L, &k]() {
					luaL_newmetatable(L, &k[0]);
					lua_setmetatable(L, -2);
				}, obj);
			}

			static int push(lua_State* L, T* obj) {
				return push_keyed(L, usertype_traits<meta::unqualified_t<T>*>::metatable(), obj);
			}
		};

		template <>
		struct pusher<detail::as_reference_tag> {
			template <typename T>
			static int push(lua_State* L, T&& obj) {
				return stack::push(L, detail::ptr(obj));
			}
		};

		template<typename T, typename>
		struct pusher {
			template <typename... Args>
			static int push(lua_State* L, Args&&... args) {
				return pusher<detail::as_value_tag<T>>{}.push(L, std::forward<Args>(args)...);
			}
		};
		
		template<typename T>
		struct pusher<T*, meta::disable_if_t<meta::all<is_container<meta::unqualified_t<T>>, meta::neg<meta::any<std::is_base_of<reference, meta::unqualified_t<T>>, std::is_base_of<stack_reference, meta::unqualified_t<T>>>>>::value>> {
			template <typename... Args>
			static int push(lua_State* L, Args&&... args) {
				return pusher<detail::as_pointer_tag<T>>{}.push(L, std::forward<Args>(args)...);
			}
		};

		template<typename T>
		struct pusher<T, std::enable_if_t<is_unique_usertype<T>::value>> {
			typedef typename unique_usertype_traits<T>::type P;
			typedef typename unique_usertype_traits<T>::actual_type Real;

			template <typename Arg, meta::enable<std::is_base_of<Real, meta::unqualified_t<Arg>>> = meta::enabler>
			static int push(lua_State* L, Arg&& arg) {
				if (unique_usertype_traits<T>::is_null(arg))
					return stack::push(L, lua_nil);
				return push_deep(L, std::forward<Arg>(arg));
			}

			template <typename Arg0, typename Arg1, typename... Args>
			static int push(lua_State* L, Arg0&& arg0, Arg0&& arg1, Args&&... args) {
				return push_deep(L, std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::forward<Args>(args)...);
			}

			template <typename... Args>
			static int push_deep(lua_State* L, Args&&... args) {
				P** pref = static_cast<P**>(lua_newuserdata(L, sizeof(P*) + sizeof(detail::special_destruct_func) + sizeof(Real)));
				detail::special_destruct_func* fx = static_cast<detail::special_destruct_func*>(static_cast<void*>(pref + 1));
				Real* mem = static_cast<Real*>(static_cast<void*>(fx + 1));
				*fx = detail::special_destruct<P, Real>;
				detail::default_construct::construct(mem, std::forward<Args>(args)...);
				*pref = unique_usertype_traits<T>::get(*mem);
				if (luaL_newmetatable(L, &usertype_traits<detail::unique_usertype<P>>::metatable()[0]) == 1) {
					set_field(L, "__gc", detail::unique_destruct<P>);
				}
				lua_setmetatable(L, -2);
				return 1;
			}
		};

		template<typename T>
		struct pusher<std::reference_wrapper<T>> {
			static int push(lua_State* L, const std::reference_wrapper<T>& t) {
				return stack::push(L, std::addressof(detail::deref(t.get())));
			}
		};

		template<typename T>
		struct pusher<T, std::enable_if_t<std::is_floating_point<T>::value>> {
			static int push(lua_State* L, const T& value) {
				lua_pushnumber(L, value);
				return 1;
			}
		};

		template<typename T>
		struct pusher<T, std::enable_if_t<meta::all<std::is_integral<T>, std::is_signed<T>>::value>> {
			static int push(lua_State* L, const T& value) {
				lua_pushinteger(L, static_cast<lua_Integer>(value));
				return 1;
			}
		};

		template<typename T>
		struct pusher<T, std::enable_if_t<std::is_enum<T>::value>> {
			static int push(lua_State* L, const T& value) {
				if (std::is_same<char, T>::value) {
					return stack::push(L, static_cast<int>(value));
				}
				return stack::push(L, static_cast<std::underlying_type_t<T>>(value));
			}
		};

		template<typename T>
		struct pusher<T, std::enable_if_t<meta::all<std::is_integral<T>, std::is_unsigned<T>>::value>> {
			static int push(lua_State* L, const T& value) {
				lua_pushinteger(L, static_cast<lua_Integer>(value));
				return 1;
			}
		};

		template<typename T>
		struct pusher<as_table_t<T>, std::enable_if_t<!meta::has_key_value_pair<meta::unqualified_t<std::remove_pointer_t<T>>>::value>> {
			static int push(lua_State* L, const as_table_t<T>& tablecont) {
				auto& cont = detail::deref(detail::unwrap(tablecont.source));
				lua_createtable(L, static_cast<int>(cont.size()), 0);
				int tableindex = lua_gettop(L);
				std::size_t index = 1;
				for (const auto& i : cont) {
#if SOL_LUA_VERSION >= 503
					int p = stack::push(L, i);
					for (int pi = 0; pi < p; ++pi) {
						lua_seti(L, tableindex, static_cast<lua_Integer>(index++));
					}
#else
					lua_pushinteger(L, static_cast<lua_Integer>(index));
					int p = stack::push(L, i);
					if (p == 1) {
						++index;
						lua_settable(L, tableindex);
					}
					else {
						int firstindex = tableindex + 1 + 1;
						for (int pi = 0; pi < p; ++pi) {
							stack::push(L, index);
							lua_pushvalue(L, firstindex);
							lua_settable(L, tableindex);
							++index;
							++firstindex;
						}
						lua_pop(L, 1 + p);
					}
#endif
				}
				// TODO: figure out a better way to do this...?
				//set_field(L, -1, cont.size());
				return 1;
			}
		};

		template<typename T>
		struct pusher<as_table_t<T>, std::enable_if_t<meta::has_key_value_pair<meta::unqualified_t<std::remove_pointer_t<T>>>::value>> {
			static int push(lua_State* L, const as_table_t<T>& tablecont) {
				auto& cont = detail::deref(detail::unwrap(tablecont.source));
				lua_createtable(L, static_cast<int>(cont.size()), 0);
				int tableindex = lua_gettop(L);
				for (const auto& pair : cont) {
					set_field(L, pair.first, pair.second, tableindex);
				}
				return 1;
			}
		};

		template<typename T>
		struct pusher<T, std::enable_if_t<std::is_base_of<reference, T>::value || std::is_base_of<stack_reference, T>::value>> {
			static int push(lua_State* L, const T& ref) {
				return ref.push(L);
			}

			static int push(lua_State* L, T&& ref) {
				return ref.push(L);
			}
		};

		template<>
		struct pusher<bool> {
			static int push(lua_State* L, bool b) {
				lua_pushboolean(L, b);
				return 1;
			}
		};

		template<>
		struct pusher<lua_nil_t> {
			static int push(lua_State* L, lua_nil_t) {
				lua_pushnil(L);
				return 1;
			}
		};

		template<>
		struct pusher<metatable_key_t> {
			static int push(lua_State* L, metatable_key_t) {
				lua_pushlstring(L, "__mt", 4);
				return 1;
			}
		};

		template<>
		struct pusher<std::remove_pointer_t<lua_CFunction>> {
			static int push(lua_State* L, lua_CFunction func, int n = 0) {
				lua_pushcclosure(L, func, n);
				return 1;
			}
		};

		template<>
		struct pusher<lua_CFunction> {
			static int push(lua_State* L, lua_CFunction func, int n = 0) {
				lua_pushcclosure(L, func, n);
				return 1;
			}
		};

		template<>
		struct pusher<c_closure> {
			static int push(lua_State* L, c_closure cc) {
				lua_pushcclosure(L, cc.c_function, cc.upvalues);
				return 1;
			}
		};

		template<typename Arg, typename... Args>
		struct pusher<closure<Arg, Args...>> {
			template <std::size_t... I, typename T>
			static int push(std::index_sequence<I...>, lua_State* L, T&& c) {
				int pushcount = multi_push(L, detail::forward_get<I>(c.upvalues)...);
				return stack::push(L, c_closure(c.c_function, pushcount));
			}

			template <typename T>
			static int push(lua_State* L, T&& c) {
				return push(std::make_index_sequence<1 + sizeof...(Args)>(), L, std::forward<T>(c));
			}
		};

		template<>
		struct pusher<void*> {
			static int push(lua_State* L, void* userdata) {
				lua_pushlightuserdata(L, userdata);
				return 1;
			}
		};

		template<>
		struct pusher<lightuserdata_value> {
			static int push(lua_State* L, lightuserdata_value userdata) {
				lua_pushlightuserdata(L, userdata);
				return 1;
			}
		};

		template<typename T>
		struct pusher<light<T>> {
			static int push(lua_State* L, light<T> l) {
				lua_pushlightuserdata(L, static_cast<void*>(l.value));
				return 1;
			}
		};

		template<typename T>
		struct pusher<user<T>> {
			template <bool with_meta = true, typename Key, typename... Args>
			static int push_with(lua_State* L, Key&& name, Args&&... args) {
				// A dumb pusher
				void* rawdata = lua_newuserdata(L, sizeof(T));
				T* data = static_cast<T*>(rawdata);
				std::allocator<T> alloc;
				alloc.construct(data, std::forward<Args>(args)...);
				if (with_meta) {
					lua_CFunction cdel = detail::user_alloc_destroy<T>;
					// Make sure we have a plain GC set for this data
					if (luaL_newmetatable(L, name) != 0) {
						lua_pushcclosure(L, cdel, 0);
						lua_setfield(L, -2, "__gc");
					}
					lua_setmetatable(L, -2);
				}
				return 1;
			}

			template <typename Arg, typename... Args, meta::disable<meta::any_same<meta::unqualified_t<Arg>, no_metatable_t, metatable_key_t>> = meta::enabler>
			static int push(lua_State* L, Arg&& arg, Args&&... args) {
				const auto name = &usertype_traits<meta::unqualified_t<T>>::user_gc_metatable()[0];
				return push_with(L, name, std::forward<Arg>(arg), std::forward<Args>(args)...);
			}

			template <typename... Args>
			static int push(lua_State* L, no_metatable_t, Args&&... args) {
				const auto name = &usertype_traits<meta::unqualified_t<T>>::user_gc_metatable()[0];
				return push_with<false>(L, name, std::forward<Args>(args)...);
			}

			template <typename Key, typename... Args>
			static int push(lua_State* L, metatable_key_t, Key&& key, Args&&... args) {
				const auto name = &key[0];
				return push_with<true>(L, name, std::forward<Args>(args)...);
			}

			static int push(lua_State* L, const user<T>& u) {
				const auto name = &usertype_traits<meta::unqualified_t<T>>::user_gc_metatable()[0];
				return push_with(L, name, u.value);
			}

			static int push(lua_State* L, user<T>&& u) {
				const auto name = &usertype_traits<meta::unqualified_t<T>>::user_gc_metatable()[0];
				return push_with(L, name, std::move(u.value));
			}

			static int push(lua_State* L, no_metatable_t, const user<T>& u) {
				const auto name = &usertype_traits<meta::unqualified_t<T>>::user_gc_metatable()[0];
				return push_with<false>(L, name, u.value);
			}

			static int push(lua_State* L, no_metatable_t, user<T>&& u) {
				const auto name = &usertype_traits<meta::unqualified_t<T>>::user_gc_metatable()[0];
				return push_with<false>(L, name, std::move(u.value));
			}
		};

		template<>
		struct pusher<userdata_value> {
			static int push(lua_State* L, userdata_value data) {
				void** ud = static_cast<void**>(lua_newuserdata(L, sizeof(void*)));
				*ud = data.value;
				return 1;
			}
		};

		template<>
		struct pusher<const char*> {
			static int push_sized(lua_State* L, const char* str, std::size_t len) {
				lua_pushlstring(L, str, len);
				return 1;
			}

			static int push(lua_State* L, const char* str) {
				if (str == nullptr)
					return stack::push(L, lua_nil);
				return push_sized(L, str, std::char_traits<char>::length(str));
			}

			static int push(lua_State* L, const char* strb, const char* stre) {
				return push_sized(L, strb, stre - strb);
			}

			static int push(lua_State* L, const char* str, std::size_t len) {
				return push_sized(L, str, len);
			}
		};

		template<size_t N>
		struct pusher<char[N]> {
			static int push(lua_State* L, const char(&str)[N]) {
				lua_pushlstring(L, str, N - 1);
				return 1;
			}

			static int push(lua_State* L, const char(&str)[N], std::size_t sz) {
				lua_pushlstring(L, str, sz);
				return 1;
			}
		};

		template <>
		struct pusher<char> {
			static int push(lua_State* L, char c) {
				const char str[2] = { c, '\0' };
				return stack::push(L, str, 1);
			}
		};

		template<>
		struct pusher<std::string> {
			static int push(lua_State* L, const std::string& str) {
				lua_pushlstring(L, str.c_str(), str.size());
				return 1;
			}

			static int push(lua_State* L, const std::string& str, std::size_t sz) {
				lua_pushlstring(L, str.c_str(), sz);
				return 1;
			}
		};

		template<>
		struct pusher<meta_function> {
			static int push(lua_State* L, meta_function m) {
				const std::string& str = name_of(m);
				lua_pushlstring(L, str.c_str(), str.size());
				return 1;
			}
		};

#ifdef SOL_CODECVT_SUPPORT
		template<>
		struct pusher<const wchar_t*> {
			static int push(lua_State* L, const wchar_t* wstr) {
				return push(L, wstr, std::char_traits<wchar_t>::length(wstr));
			}

			static int push(lua_State* L, const wchar_t* wstr, std::size_t sz) {
				return push(L, wstr, wstr + sz);
			}

			static int push(lua_State* L, const wchar_t* strb, const wchar_t* stre) {
				if (sizeof(wchar_t) == 2) {
					static std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
					std::string u8str = convert.to_bytes(strb, stre);
					return stack::push(L, u8str);
				}
				static std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
				std::string u8str = convert.to_bytes(strb, stre);
				return stack::push(L, u8str);
			}
		};

		template<>
		struct pusher<const char16_t*> {
			static int push(lua_State* L, const char16_t* u16str) {
				return push(L, u16str, std::char_traits<char16_t>::length(u16str));
			}

			static int push(lua_State* L, const char16_t* u16str, std::size_t sz) {
				return push(L, u16str, u16str + sz);
			}

			static int push(lua_State* L, const char16_t* strb, const char16_t* stre) {
#ifdef _MSC_VER
				static std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t> convert;
				std::string u8str = convert.to_bytes(reinterpret_cast<const int16_t*>(strb), reinterpret_cast<const int16_t*>(stre));
#else
				static std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
				std::string u8str = convert.to_bytes(strb, stre);
#endif // VC++ is a shit
				return stack::push(L, u8str);
			}
		};

		template<>
		struct pusher<const char32_t*> {
			static int push(lua_State* L, const char32_t* u32str) {
				return push(L, u32str, u32str + std::char_traits<char32_t>::length(u32str));
			}

			static int push(lua_State* L, const char32_t* u32str, std::size_t sz) {
				return push(L, u32str, u32str + sz);
			}

			static int push(lua_State* L, const char32_t* strb, const char32_t* stre) {
#ifdef _MSC_VER
				static std::wstring_convert<std::codecvt_utf8<int32_t>, int32_t> convert;
				std::string u8str = convert.to_bytes(reinterpret_cast<const int32_t*>(strb), reinterpret_cast<const int32_t*>(stre));
#else
				static std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
				std::string u8str = convert.to_bytes(strb, stre);
#endif // VC++ is a shit
				return stack::push(L, u8str);
			}
		};

		template<size_t N>
		struct pusher<wchar_t[N]> {
			static int push(lua_State* L, const wchar_t(&str)[N]) {
				return push(L, str, N - 1);
			}

			static int push(lua_State* L, const wchar_t(&str)[N], std::size_t sz) {
				return stack::push_specific<const wchar_t*>(L, str, str + sz);
			}
		};

		template<size_t N>
		struct pusher<char16_t[N]> {
			static int push(lua_State* L, const char16_t(&str)[N]) {
				return push(L, str, N - 1);
			}

			static int push(lua_State* L, const char16_t(&str)[N], std::size_t sz) {
				return stack::push_specific<const char16_t*>(L, str, str + sz);
			}
		};

		template<size_t N>
		struct pusher<char32_t[N]> {
			static int push(lua_State* L, const char32_t(&str)[N]) {
				return push(L, str, N - 1);
			}

			static int push(lua_State* L, const char32_t(&str)[N], std::size_t sz) {
				return stack::push_specific<const char32_t*>(L, str, str + sz);
			}
		};

		template <>
		struct pusher<wchar_t> {
			static int push(lua_State* L, wchar_t c) {
				const wchar_t str[2] = { c, '\0' };
				return stack::push(L, str, 1);
			}
		};

		template <>
		struct pusher<char16_t> {
			static int push(lua_State* L, char16_t c) {
				const char16_t str[2] = { c, '\0' };
				return stack::push(L, str, 1);
			}
		};

		template <>
		struct pusher<char32_t> {
			static int push(lua_State* L, char32_t c) {
				const char32_t str[2] = { c, '\0' };
				return stack::push(L, str, 1);
			}
		};

		template<>
		struct pusher<std::wstring> {
			static int push(lua_State* L, const std::wstring& wstr) {
				return push(L, wstr.data(), wstr.size());
			}

			static int push(lua_State* L, const std::wstring& wstr, std::size_t sz) {
				return stack::push(L, wstr.data(), wstr.data() + sz);
			}
		};

		template<>
		struct pusher<std::u16string> {
			static int push(lua_State* L, const std::u16string& u16str) {
				return push(L, u16str, u16str.size());
			}
			
			static int push(lua_State* L, const std::u16string& u16str, std::size_t sz) {
				return stack::push(L, u16str.data(), u16str.data() + sz);
			}
		};

		template<>
		struct pusher<std::u32string> {
			static int push(lua_State* L, const std::u32string& u32str) {
				return push(L, u32str, u32str.size());
			}

			static int push(lua_State* L, const std::u32string& u32str, std::size_t sz) {
				return stack::push(L, u32str.data(), u32str.data() + sz);
			}
		};
#endif // codecvt Header Support

		template<typename... Args>
		struct pusher<std::tuple<Args...>> {
			template <std::size_t... I, typename T>
			static int push(std::index_sequence<I...>, lua_State* L, T&& t) {
				int pushcount = 0;
				(void)detail::swallow{ 0, (pushcount += stack::push(L,
					  detail::forward_get<I>(t)
				), 0)... };
				return pushcount;
			}

			template <typename T>
			static int push(lua_State* L, T&& t) {
				return push(std::index_sequence_for<Args...>(), L, std::forward<T>(t));
			}
		};

		template<typename A, typename B>
		struct pusher<std::pair<A, B>> {
			template <typename T>
			static int push(lua_State* L, T&& t) {
				int pushcount = stack::push(L, detail::forward_get<0>(t));
				pushcount += stack::push(L, detail::forward_get<1>(t));
				return pushcount;
			}
		};

		template<typename O>
		struct pusher<optional<O>> {
			template <typename T>
			static int push(lua_State* L, T&& t) {
				if (t == nullopt) {
					return stack::push(L, nullopt);
				}
				return stack::push(L, t.value());
			}
		};

		template<>
		struct pusher<nullopt_t> {
			static int push(lua_State* L, nullopt_t) {
				return stack::push(L, lua_nil);
			}
		};

		template<>
		struct pusher<std::nullptr_t> {
			static int push(lua_State* L, std::nullptr_t) {
				return stack::push(L, lua_nil);
			}
		};

		template<>
		struct pusher<this_state> {
			static int push(lua_State*, const this_state&) {
				return 0;
			}
		};
	} // stack
} // sol

#endif // SOL_STACK_PUSH_HPP
