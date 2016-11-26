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

#ifndef SOL_STACK_GET_HPP
#define SOL_STACK_GET_HPP

#include "stack_core.hpp"
#include "usertype_traits.hpp"
#include "inheritance.hpp"
#include "overload.hpp"
#include "error.hpp"
#include <memory>
#include <functional>
#include <utility>
#ifdef SOL_CODECVT_SUPPORT
#include <codecvt>
#include <locale>
#endif

namespace sol {
	namespace stack {

		template<typename T, typename>
		struct getter {
			static T& get(lua_State* L, int index, record& tracking) {
				return getter<sol::detail::as_value_tag<T>>{}.get(L, index, tracking);
			}
		};

		template<typename T>
		struct getter<T, std::enable_if_t<std::is_floating_point<T>::value>> {
			static T get(lua_State* L, int index, record& tracking) {
				tracking.use(1);
				return static_cast<T>(lua_tonumber(L, index));
			}
		};

		template<typename T>
		struct getter<T, std::enable_if_t<meta::all<std::is_integral<T>, std::is_signed<T>>::value>> {
			static T get(lua_State* L, int index, record& tracking) {
				tracking.use(1);
				return static_cast<T>(lua_tointeger(L, index));
			}
		};

		template<typename T>
		struct getter<T, std::enable_if_t<meta::all<std::is_integral<T>, std::is_unsigned<T>>::value>> {
			static T get(lua_State* L, int index, record& tracking) {
				tracking.use(1);
				return static_cast<T>(lua_tointeger(L, index));
			}
		};

		template<typename T>
		struct getter<T, std::enable_if_t<std::is_enum<T>::value>> {
			static T get(lua_State* L, int index, record& tracking) {
				tracking.use(1);
				return static_cast<T>(lua_tointegerx(L, index, nullptr));
			}
		};

		template<typename T>
		struct getter<as_table_t<T>, std::enable_if_t<!meta::has_key_value_pair<meta::unqualified_t<T>>::value>> {
			static T get(lua_State* L, int index, record& tracking) {
				typedef typename T::value_type V;
				tracking.use(1);

				index = lua_absindex(L, index);
				T arr;
				get_field<false, true>(L, static_cast<lua_Integer>(-1), index);
				int isnum;
				std::size_t sizehint = static_cast<std::size_t>(lua_tointegerx(L, -1, &isnum));
				if (isnum != 0) {
					detail::reserve(arr, sizehint);
				}
				lua_pop(L, 1);
#if SOL_LUA_VERSION >= 503
				// This method is HIGHLY performant over regular table iteration thanks to the Lua API changes in 5.3
				for (lua_Integer i = 0; ; i += lua_size<V>::value, lua_pop(L, lua_size<V>::value)) {
					for (int vi = 0; vi < lua_size<V>::value; ++vi) {
						type t = static_cast<type>(lua_geti(L, index, i + vi));
						if (t == type::nil) {
							if (i == 0) {
								continue;
							}
							else {
								lua_pop(L, (vi + 1));
								return arr;
							}
						}
					}
					arr.push_back(stack::get<V>(L, -lua_size<V>::value));
				}
#else
				// Zzzz slower but necessary thanks to the lower version API and missing functions qq
				for (lua_Integer i = 0; ; i += lua_size<V>::value, lua_pop(L, lua_size<V>::value)) {
					for (int vi = 0; vi < lua_size<V>::value; ++vi) {
						lua_pushinteger(L, i);
						lua_gettable(L, index);
						type t = type_of(L, -1);
						if (t == type::nil) {
							if (i == 0) {
								continue;
							}
							else {
								lua_pop(L, (vi + 1));
								return arr;
							}
						}
					}
					arr.push_back(stack::get<V>(L, -1));
				}
#endif
				return arr;
			}
		};

		template<typename T>
		struct getter<as_table_t<T>, std::enable_if_t<meta::has_key_value_pair<meta::unqualified_t<T>>::value>> {
			static T get(lua_State* L, int index, record& tracking) {
				typedef typename T::value_type P;
				typedef typename P::first_type K;
				typedef typename P::second_type V;
				tracking.use(1);

				T associative;
				index = lua_absindex(L, index);
				lua_pushnil(L);
				while (lua_next(L, index) != 0) {
					decltype(auto) key = stack::check_get<K>(L, -2);
					if (!key) {
						lua_pop(L, 1);
						continue;
					}
					associative.emplace(std::forward<decltype(*key)>(*key), stack::get<V>(L, -1));
					lua_pop(L, 1);
				}
				return associative;
			}
		};

		template<typename T>
		struct getter<T, std::enable_if_t<std::is_base_of<reference, T>::value || std::is_base_of<stack_reference, T>::value>> {
			static T get(lua_State* L, int index, record& tracking) {
				tracking.use(1);
				return T(L, index);
			}
		};

		template<>
		struct getter<userdata_value> {
			static userdata_value get(lua_State* L, int index, record& tracking) {
				tracking.use(1);
				return userdata_value(lua_touserdata(L, index));
			}
		};

		template<>
		struct getter<lightuserdata_value> {
			static lightuserdata_value get(lua_State* L, int index, record& tracking) {
				tracking.use(1);
				return lightuserdata_value(lua_touserdata(L, index));
			}
		};

		template<typename T>
		struct getter<light<T>> {
			static light<T> get(lua_State* L, int index, record& tracking) {
				tracking.use(1);
				return light<T>(static_cast<T*>(lua_touserdata(L, index)));
			}
		};

		template<typename T>
		struct getter<user<T>> {
			static T& get(lua_State* L, int index, record& tracking) {
				tracking.use(1);
				return *static_cast<T*>(lua_touserdata(L, index));
			}
		};

		template<typename T>
		struct getter<user<T*>> {
			static T* get(lua_State* L, int index, record& tracking) {
				tracking.use(1);
				return static_cast<T*>(lua_touserdata(L, index));
			}
		};

		template<>
		struct getter<type> {
			static type get(lua_State *L, int index, record& tracking) {
				tracking.use(1);
				return static_cast<type>(lua_type(L, index));
			}
		};

		template<>
		struct getter<bool> {
			static bool get(lua_State* L, int index, record& tracking) {
				tracking.use(1);
				return lua_toboolean(L, index) != 0;
			}
		};

		template<>
		struct getter<std::string> {
			static std::string get(lua_State* L, int index, record& tracking) {
				tracking.use(1);
				std::size_t len;
				auto str = lua_tolstring(L, index, &len);
				return std::string( str, len );
			}
		};

		template <>
		struct getter<string_detail::string_shim> {
			string_detail::string_shim get(lua_State* L, int index, record& tracking) {
				tracking.use(1);
				size_t len;
				const char* p = lua_tolstring(L, index, &len);
				return string_detail::string_shim(p, len);
			}
		};

		template<>
		struct getter<const char*> {
			static const char* get(lua_State* L, int index, record& tracking) {
				tracking.use(1);
				return lua_tostring(L, index);
			}
		};
		
		template<>
		struct getter<char> {
			static char get(lua_State* L, int index, record& tracking) {
				tracking.use(1);
				size_t len;
				auto str = lua_tolstring(L, index, &len);
				return len > 0 ? str[0] : '\0';
			}
		};

#ifdef SOL_CODECVT_SUPPORT
		template<>
		struct getter<std::wstring> {
			static std::wstring get(lua_State* L, int index, record& tracking) {
				tracking.use(1);
				size_t len;
				auto str = lua_tolstring(L, index, &len);
				if (len < 1)
					return std::wstring();
				if (sizeof(wchar_t) == 2) {
					std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> convert;
					std::wstring r = convert.from_bytes(str, str + len);
#ifdef __MINGW32__
					// Fuck you, MinGW, and fuck you libstdc++ for introducing this absolutely asinine bug
					// https://sourceforge.net/p/mingw-w64/bugs/538/
					// http://chat.stackoverflow.com/transcript/message/32271369#32271369
					for (auto& c : r) {
                        uint8_t* b = reinterpret_cast<uint8_t*>(&c);
						std::swap(b[0], b[1]);
					}
#endif 
					return r;
				}
				std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
				std::wstring r = convert.from_bytes(str, str + len);
				return r;
			}
		};

		template<>
		struct getter<std::u16string> {
			static std::u16string get(lua_State* L, int index, record& tracking) {
				tracking.use(1);
				size_t len;
				auto str = lua_tolstring(L, index, &len);
				if (len < 1)
					return std::u16string();
#ifdef _MSC_VER
				std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t> convert;
				auto intd = convert.from_bytes(str, str + len);
				std::u16string r(intd.size(), '\0');
				std::memcpy(&r[0], intd.data(), intd.size() * sizeof(char16_t));
#else
				std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
				std::u16string r = convert.from_bytes(str, str + len);
#endif // VC++ is a shit
				return r;
			}
		};

		template<>
		struct getter<std::u32string> {
			static std::u32string get(lua_State* L, int index, record& tracking) {
				tracking.use(1);
				size_t len;
				auto str = lua_tolstring(L, index, &len);
				if (len < 1)
					return std::u32string();
#ifdef _MSC_VER
				std::wstring_convert<std::codecvt_utf8<int32_t>, int32_t> convert;
				auto intd = convert.from_bytes(str, str + len);
				std::u32string r(intd.size(), '\0');
				std::memcpy(&r[0], intd.data(), r.size() * sizeof(char32_t));
#else
				std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> convert;
				std::u32string r = convert.from_bytes(str, str + len);
#endif // VC++ is a shit
				return r;
			}
		};

		template<>
		struct getter<wchar_t> {
			static wchar_t get(lua_State* L, int index, record& tracking) {
				auto str = getter<std::wstring>{}.get(L, index, tracking);
				return str.size() > 0 ? str[0] : wchar_t(0);
			}
		};

		template<>
		struct getter<char16_t> {
			static char16_t get(lua_State* L, int index, record& tracking) {
				auto str = getter<std::u16string>{}.get(L, index, tracking);
				return str.size() > 0 ? str[0] : char16_t(0);
			}
		};

		template<>
		struct getter<char32_t> {
			static char32_t get(lua_State* L, int index, record& tracking) {
				auto str = getter<std::u32string>{}.get(L, index, tracking);
				return str.size() > 0 ? str[0] : char32_t(0);
			}
		};
#endif // codecvt header support

		template<>
		struct getter<meta_function> {
			static meta_function get(lua_State *L, int index, record& tracking) {
				tracking.use(1);
				const char* name = getter<const char*>{}.get(L, index, tracking);
				for (std::size_t i = 0; i < meta_function_names.size(); ++i)
					if (meta_function_names[i] == name)
						return static_cast<meta_function>(i);
				return meta_function::construct;
			}
		};

		template<>
		struct getter<nil_t> {
			static nil_t get(lua_State*, int, record& tracking) {
				tracking.use(1);
				return nil;
			}
		};

		template<>
		struct getter<std::nullptr_t> {
			static std::nullptr_t get(lua_State*, int, record& tracking) {
				tracking.use(1);
				return nullptr;
			}
		};

		template<>
		struct getter<nullopt_t> {
			static nullopt_t get(lua_State*, int, record& tracking) {
				tracking.use(1);
				return nullopt;
			}
		};

		template<>
		struct getter<this_state> {
			static this_state get(lua_State* L, int, record& tracking) {
				tracking.use(0);
				return this_state{ L };
			}
		};

		template<>
		struct getter<lua_CFunction> {
			static lua_CFunction get(lua_State* L, int index, record& tracking) {
				tracking.use(1);
				return lua_tocfunction(L, index);
			}
		};

		template<>
		struct getter<c_closure> {
			static c_closure get(lua_State* L, int index, record& tracking) {
				tracking.use(1);
				return c_closure(lua_tocfunction(L, index), -1);
			}
		};

		template<>
		struct getter<error> {
			static error get(lua_State* L, int index, record& tracking) {
				tracking.use(1);
				size_t sz = 0;
				const char* err = lua_tolstring(L, index, &sz);
				if (err == nullptr) {
					return error(detail::direct_error, "");
				}
				return error(detail::direct_error, std::string(err, sz));
			}
		};

		template<>
		struct getter<void*> {
			static void* get(lua_State* L, int index, record& tracking) {
				tracking.use(1);
				return lua_touserdata(L, index);
			}
		};

		template<typename T>
		struct getter<detail::as_value_tag<T>> {
			static T* get_no_nil(lua_State* L, int index, record& tracking) {
				tracking.use(1);
				void** pudata = static_cast<void**>(lua_touserdata(L, index));
				void* udata = *pudata;
				return get_no_nil_from(L, udata, index, tracking);
			}

			static T* get_no_nil_from(lua_State* L, void* udata, int index, record&) {
				if (detail::has_derived<T>::value && luaL_getmetafield(L, index, &detail::base_class_cast_key()[0]) != 0) {
					void* basecastdata = lua_touserdata(L, -1);
					detail::inheritance_cast_function ic = (detail::inheritance_cast_function)basecastdata;
					// use the casting function to properly adjust the pointer for the desired T
					udata = ic(udata, detail::id_for<T>::value);
					lua_pop(L, 1);
				}
				T* obj = static_cast<T*>(udata);
				return obj;
			}
			
			static T& get(lua_State* L, int index, record& tracking) {
				return *get_no_nil(L, index, tracking);
			}
		};

		template<typename T>
		struct getter<detail::as_pointer_tag<T>> {
			static T* get(lua_State* L, int index, record& tracking) {
				type t = type_of(L, index);
				if (t == type::nil) {
					tracking.use(1);
					return nullptr;
				}
				return getter<detail::as_value_tag<T>>::get_no_nil(L, index, tracking);
			}
		};

		template<typename T>
		struct getter<non_null<T*>> {
			static T* get(lua_State* L, int index, record& tracking) {
				return getter<detail::as_value_tag<T>>::get_no_nil(L, index, tracking);
			}
		};

		template<typename T>
		struct getter<T&> {
			static T& get(lua_State* L, int index, record& tracking) {
				return getter<detail::as_value_tag<T>>::get(L, index, tracking);
			}
		};

		template<typename T>
		struct getter<std::reference_wrapper<T>> {
			static T& get(lua_State* L, int index, record& tracking) {
				return getter<T&>{}.get(L, index, tracking);
			}
		};

		template<typename T>
		struct getter<T*> {
			static T* get(lua_State* L, int index, record& tracking) {
				return getter<detail::as_pointer_tag<T>>::get(L, index, tracking);
			}
		};

		template<typename T>
		struct getter<T, std::enable_if_t<is_unique_usertype<T>::value>> {
			typedef typename unique_usertype_traits<T>::type P;
			typedef typename unique_usertype_traits<T>::actual_type Real;

			static Real& get(lua_State* L, int index, record& tracking) {
				tracking.use(1);
				P** pref = static_cast<P**>(lua_touserdata(L, index));
				detail::special_destruct_func* fx = static_cast<detail::special_destruct_func*>(static_cast<void*>(pref + 1));
				Real* mem = static_cast<Real*>(static_cast<void*>(fx + 1));
				return *mem;
			}
		};

		template<typename... Args>
		struct getter<std::tuple<Args...>> {
			typedef std::tuple<decltype(stack::get<Args>(nullptr, 0))...> R;
			
			template <typename... TArgs>
			static R apply(std::index_sequence<>, lua_State*, int, record&, TArgs&&... args) {
				// Fuck you too, VC++
				return R{std::forward<TArgs>(args)...};
			}
			
			template <std::size_t I, std::size_t... Ix, typename... TArgs>
			static R apply(std::index_sequence<I, Ix...>, lua_State* L, int index, record& tracking, TArgs&&... args) {
				// Fuck you too, VC++
				typedef std::tuple_element_t<I, std::tuple<Args...>> T;
				return apply(std::index_sequence<Ix...>(), L, index, tracking, std::forward<TArgs>(args)..., stack::get<T>(L, index + tracking.used, tracking));
			}

			static R get(lua_State* L, int index, record& tracking) {
				return apply(std::make_index_sequence<sizeof...(Args)>(), L, index, tracking);
			}
		};

		template<typename A, typename B>
		struct getter<std::pair<A, B>> {
			static decltype(auto) get(lua_State* L, int index, record& tracking) {
				return std::pair<decltype(stack::get<A>(L, index)), decltype(stack::get<B>(L, index))>{stack::get<A>(L, index, tracking), stack::get<B>(L, index + tracking.used, tracking)};
			}
		};

	} // stack
} // sol

#endif // SOL_STACK_GET_HPP
