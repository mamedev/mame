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

#ifndef SOL_TYPES_HPP
#define SOL_TYPES_HPP

#include "optional.hpp"
#include "compatibility.hpp"
#include "traits.hpp"
#include "string_shim.hpp"
#include <array>
#include <string>

namespace sol {
	namespace detail {
#ifdef SOL_NO_EXCEPTIONS
		template <lua_CFunction f>
		int static_trampoline(lua_State* L) {
			return f(L);
		}

		template <typename Fx, typename... Args>
		int trampoline(lua_State* L, Fx&& f, Args&&... args) {
			return f(L, std::forward<Args>(args)...);
		}

		inline int c_trampoline(lua_State* L, lua_CFunction f) {
			return trampoline(L, f);
		}
#else
		template <lua_CFunction f>
		int static_trampoline(lua_State* L) {
			try {
				return f(L);
			}
			catch (const char *s) {
				lua_pushstring(L, s);
			}
			catch (const std::exception& e) {
				lua_pushstring(L, e.what());
			}
#if !defined(SOL_EXCEPTIONS_SAFE_PROPAGATION)
			catch (...) {
				lua_pushstring(L, "caught (...) exception");
			}
#endif
			return lua_error(L);
		}

		template <typename Fx, typename... Args>
		int trampoline(lua_State* L, Fx&& f, Args&&... args) {
			try {
				return f(L, std::forward<Args>(args)...);
			}
			catch (const char *s) {
				lua_pushstring(L, s);
			}
			catch (const std::exception& e) {
				lua_pushstring(L, e.what());
			}
#if !defined(SOL_EXCEPTIONS_SAFE_PROPAGATION)
			catch (...) {
				lua_pushstring(L, "caught (...) exception");
			}
#endif
			return lua_error(L);
		}

		inline int c_trampoline(lua_State* L, lua_CFunction f) {
			return trampoline(L, f);
		}
#endif // Exceptions vs. No Exceptions

		template <typename T>
		struct unique_usertype {};

		template <typename T>
		struct implicit_wrapper {
			T& item;
			implicit_wrapper(T* item) : item(*item) {}
			implicit_wrapper(T& item) : item(item) {}
			operator T& () {
				return item;
			}
			operator T* () {
				return std::addressof(item);
			}
		};
	} // detail

	struct lua_nil_t {};
	const lua_nil_t lua_nil{};
	inline bool operator==(lua_nil_t, lua_nil_t) { return true; }
	inline bool operator!=(lua_nil_t, lua_nil_t) { return false; }
#ifndef __OBJC__
	typedef lua_nil_t nil_t;
	const nil_t nil{};
#endif

	struct metatable_key_t {};
	const metatable_key_t metatable_key = {};

	struct no_metatable_t {};
	const no_metatable_t no_metatable = {};

	typedef std::remove_pointer_t<lua_CFunction> lua_r_CFunction;

	template <typename T>
	struct unique_usertype_traits {
		typedef T type;
		typedef T actual_type;
		static const bool value = false;

		template <typename U>
		static bool is_null(U&&) {
			return false;
		}

		template <typename U>
		static auto get(U&& value) {
			return std::addressof(detail::deref(value));
		}
	};

	template <typename T>
	struct unique_usertype_traits<std::shared_ptr<T>> {
		typedef T type;
		typedef std::shared_ptr<T> actual_type;
		static const bool value = true;

		static bool is_null(const actual_type& p) {
			return p == nullptr;
		}

		static type* get(const actual_type& p) {
			return p.get();
		}
	};

	template <typename T, typename D>
	struct unique_usertype_traits<std::unique_ptr<T, D>> {
		typedef T type;
		typedef std::unique_ptr<T, D> actual_type;
		static const bool value = true;

		static bool is_null(const actual_type& p) {
			return p == nullptr;
		}

		static type* get(const actual_type& p) {
			return p.get();
		}
	};

	template <typename T>
	struct non_null {};

	template <typename... Args>
	struct function_sig {};

	struct upvalue_index {
		int index;
		upvalue_index(int idx) : index(lua_upvalueindex(idx)) {}
		operator int() const { return index; }
	};

	struct raw_index {
		int index;
		raw_index(int i) : index(i) {}
		operator int() const { return index; }
	};

	struct absolute_index {
		int index;
		absolute_index(lua_State* L, int idx) : index(lua_absindex(L, idx)) {}
		operator int() const { return index; }
	};

	struct ref_index {
		int index;
		ref_index(int idx) : index(idx) {}
		operator int() const { return index; }
	};

	struct lightuserdata_value {
		void* value;
		lightuserdata_value(void* data) : value(data) {}
		operator void*() const { return value; }
	};

	struct userdata_value {
		void* value;
		userdata_value(void* data) : value(data) {}
		operator void*() const { return value; }
	};

	template <typename L>
	struct light {
		L* value;

		light(L& x) : value(std::addressof(x)) {}
		light(L* x) : value(x) {}
		light(void* x) : value(static_cast<L*>(x)) {}
		operator L* () const { return value; }
		operator L& () const { return *value; }
	};

	template <typename T>
	auto make_light(T& l) {
		typedef meta::unwrapped_t<std::remove_pointer_t<std::remove_pointer_t<T>>> L;
		return light<L>(l);
	}

	template <typename U>
	struct user {
		U value;

		user(U x) : value(std::move(x)) {}
		operator U* () { return std::addressof(value); }
		operator U& () { return value; }
		operator const U& () const { return value; }
	};

	template <typename T>
	auto make_user(T&& u) {
		typedef meta::unwrapped_t<meta::unqualified_t<T>> U;
		return user<U>(std::forward<T>(u));
	}

	template <typename T>
	struct metatable_registry_key {
		T key;

		metatable_registry_key(T key) : key(std::forward<T>(key)) {}
	};

	template <typename T>
	auto meta_registry_key(T&& key) {
		typedef meta::unqualified_t<T> K;
		return metatable_registry_key<K>(std::forward<T>(key));
	}

	template <typename... Upvalues>
	struct closure {
		lua_CFunction c_function;
		std::tuple<Upvalues...> upvalues;
		closure(lua_CFunction f, Upvalues... targetupvalues) : c_function(f), upvalues(std::forward<Upvalues>(targetupvalues)...) {}
	};

	template <>
	struct closure<> {
		lua_CFunction c_function;
		int upvalues;
		closure(lua_CFunction f, int upvalue_count = 0) : c_function(f), upvalues(upvalue_count) {}
	};

	typedef closure<> c_closure;

	template <typename... Args>
	closure<Args...> make_closure(lua_CFunction f, Args&&... args) {
		return closure<Args...>(f, std::forward<Args>(args)...);
	}

	template <typename Sig, typename... Ps>
	struct function_arguments {
		std::tuple<Ps...> arguments;
		template <typename Arg, typename... Args, meta::disable<std::is_same<meta::unqualified_t<Arg>, function_arguments>> = meta::enabler>
		function_arguments(Arg&& arg, Args&&... args) : arguments(std::forward<Arg>(arg), std::forward<Args>(args)...) {}
	};

	template <typename Sig = function_sig<>, typename... Args>
	auto as_function(Args&&... args) {
		return function_arguments<Sig, std::decay_t<Args>...>(std::forward<Args>(args)...);
	}

	template <typename Sig = function_sig<>, typename... Args>
	auto as_function_reference(Args&&... args) {
		return function_arguments<Sig, Args...>(std::forward<Args>(args)...);
	}

	template <typename T>
	struct as_table_t {
		T source;
		template <typename... Args>
		as_table_t(Args&&... args) : source(std::forward<Args>(args)...) {}

		operator std::add_lvalue_reference_t<T> () {
			return source;
		}
	};

	template <typename T>
	as_table_t<T> as_table(T&& container) {
		return as_table_t<T>(std::forward<T>(container));
	}

	struct this_state {
		lua_State* L;
		operator lua_State* () const {
			return L;
		}
		lua_State* operator-> () const {
			return L;
		}
	};

	enum class call_syntax {
		dot = 0,
		colon = 1
	};

	enum class call_status : int {
		ok = LUA_OK,
		yielded = LUA_YIELD,
		runtime = LUA_ERRRUN,
		memory = LUA_ERRMEM,
		handler = LUA_ERRERR,
		gc = LUA_ERRGCMM
	};

	enum class thread_status : int {
		ok = LUA_OK,
		yielded = LUA_YIELD,
		runtime = LUA_ERRRUN,
		memory = LUA_ERRMEM,
		gc = LUA_ERRGCMM,
		handler = LUA_ERRERR,
		dead = -1,
	};

	enum class load_status : int {
		ok = LUA_OK,
		syntax = LUA_ERRSYNTAX,
		memory = LUA_ERRMEM,
		gc = LUA_ERRGCMM,
		file = LUA_ERRFILE,
	};

	enum class type : int {
		none = LUA_TNONE,
		lua_nil = LUA_TNIL,
#ifndef __OBJC__
		nil = lua_nil,
#endif // Objective C++ Keyword
		string = LUA_TSTRING,
		number = LUA_TNUMBER,
		thread = LUA_TTHREAD,
		boolean = LUA_TBOOLEAN,
		function = LUA_TFUNCTION,
		userdata = LUA_TUSERDATA,
		lightuserdata = LUA_TLIGHTUSERDATA,
		table = LUA_TTABLE,
		poly = none | lua_nil | string | number | thread |
		table | boolean | function | userdata | lightuserdata
	};

	enum class meta_function {
		construct,
		index,
		new_index,
		mode,
		call,
		call_function = call,
		metatable,
		to_string,
		length,
		unary_minus,
		addition,
		subtraction,
		multiplication,
		division,
		modulus,
		power_of,
		involution = power_of,
		concatenation,
		equal_to,
		less_than,
		less_than_or_equal_to,
		garbage_collect,
		floor_division,
		bitwise_left_shift,
		bitwise_right_shift,
		bitwise_not,
		bitwise_and,
		bitwise_or,
		bitwise_xor,
		pairs,
		next
	};

	typedef meta_function meta_method;

	const std::array<std::string, 2> meta_variable_names = { {
		"__index",
		"__newindex",
	} };

	const std::array<std::string, 29> meta_function_names = { {
		"new",
		"__index",
		"__newindex",
		"__mode",
		"__call",
		"__mt",
		"__tostring",
		"__len",
		"__unm",
		"__add",
		"__sub",
		"__mul",
		"__div",
		"__mod",
		"__pow",
		"__concat",
		"__eq",
		"__lt",
		"__le",
		"__gc",
		
		"__idiv",
		"__shl",
		"__shr",
		"__bnot",
		"__band",
		"__bor",
		"__bxor",

		"__pairs",
		"__next"
	} };

	inline const std::string& name_of(meta_function mf) {
		return meta_function_names[static_cast<int>(mf)];
	}

	inline type type_of(lua_State* L, int index) {
		return static_cast<type>(lua_type(L, index));
	}

	inline int type_panic(lua_State* L, int index, type expected, type actual) {
		return luaL_error(L, "stack index %d, expected %s, received %s", index,
			expected == type::poly ? "anything" : lua_typename(L, static_cast<int>(expected)),
			expected == type::poly ? "anything" : lua_typename(L, static_cast<int>(actual))
		);
	}

	// Specify this function as the handler for lua::check if you know there's nothing wrong
	inline int no_panic(lua_State*, int, type, type) noexcept {
		return 0;
	}

	inline void type_error(lua_State* L, int expected, int actual) {
		luaL_error(L, "expected %s, received %s", lua_typename(L, expected), lua_typename(L, actual));
	}

	inline void type_error(lua_State* L, type expected, type actual) {
		type_error(L, static_cast<int>(expected), static_cast<int>(actual));
	}

	inline void type_assert(lua_State* L, int index, type expected, type actual) {
		if (expected != type::poly && expected != actual) {
			type_panic(L, index, expected, actual);
		}
	}

	inline void type_assert(lua_State* L, int index, type expected) {
		type actual = type_of(L, index);
		type_assert(L, index, expected, actual);
	}

	inline std::string type_name(lua_State* L, type t) {
		return lua_typename(L, static_cast<int>(t));
	}

	class reference;
	class stack_reference;
	template <typename Table, typename Key>
	struct proxy;
	template<typename T>
	class usertype;
	template <bool, typename T>
	class basic_table_core;
	template <bool b>
	using table_core = basic_table_core<b, reference>;
	template <bool b>
	using stack_table_core = basic_table_core<b, stack_reference>;
	typedef table_core<false> table;
	typedef table_core<true> global_table;
	typedef stack_table_core<false> stack_table;
	typedef stack_table_core<true> stack_global_table;
	template <typename T>
	class basic_function;
	template <typename T>
	class basic_protected_function;
	using function = basic_function<reference>;
	using protected_function = basic_protected_function<reference>;
	using stack_function = basic_function<stack_reference>;
	using stack_protected_function = basic_protected_function<stack_reference>;
	template <typename base_t>
	class basic_object;
	template <typename base_t>
	class basic_userdata;
	template <typename base_t>
	class basic_lightuserdata;
	struct variadic_args;
	using object = basic_object<reference>;
	using stack_object = basic_object<stack_reference>;
	using userdata = basic_userdata<reference>;
	using stack_userdata = basic_userdata<stack_reference>;
	using lightuserdata = basic_lightuserdata<reference>;
	using stack_lightuserdata = basic_lightuserdata<stack_reference>;
	class coroutine;
	class thread;
	struct variadic_args;
	struct this_state;

	namespace detail {
		template <typename T, typename = void>
		struct lua_type_of : std::integral_constant<type, type::userdata> {};

		template <>
		struct lua_type_of<std::string> : std::integral_constant<type, type::string> {};

		template <>
		struct lua_type_of<std::wstring> : std::integral_constant<type, type::string> {};

		template <>
		struct lua_type_of<std::u16string> : std::integral_constant<type, type::string> {};

		template <>
		struct lua_type_of<std::u32string> : std::integral_constant<type, type::string> {};

		template <std::size_t N>
		struct lua_type_of<char[N]> : std::integral_constant<type, type::string> {};

		template <std::size_t N>
		struct lua_type_of<wchar_t[N]> : std::integral_constant<type, type::string> {};

		template <std::size_t N>
		struct lua_type_of<char16_t[N]> : std::integral_constant<type, type::string> {};

		template <std::size_t N>
		struct lua_type_of<char32_t[N]> : std::integral_constant<type, type::string> {};

		template <>
		struct lua_type_of<char> : std::integral_constant<type, type::string> {};

		template <>
		struct lua_type_of<wchar_t> : std::integral_constant<type, type::string> {};

		template <>
		struct lua_type_of<char16_t> : std::integral_constant<type, type::string> {};

		template <>
		struct lua_type_of<char32_t> : std::integral_constant<type, type::string> {};

		template <>
		struct lua_type_of<const char*> : std::integral_constant<type, type::string> {};

		template <>
		struct lua_type_of<const char16_t*> : std::integral_constant<type, type::string> {};

		template <>
		struct lua_type_of<const char32_t*> : std::integral_constant<type, type::string> {};

		template <>
		struct lua_type_of<string_detail::string_shim> : std::integral_constant<type, type::string> {};

		template <>
		struct lua_type_of<bool> : std::integral_constant<type, type::boolean> {};

		template <>
		struct lua_type_of<lua_nil_t> : std::integral_constant<type, type::lua_nil> { };

		template <>
		struct lua_type_of<nullopt_t> : std::integral_constant<type, type::lua_nil> { };

		template <>
		struct lua_type_of<std::nullptr_t> : std::integral_constant<type, type::lua_nil> { };

		template <>
		struct lua_type_of<sol::error> : std::integral_constant<type, type::string> { };

		template <bool b, typename Base>
		struct lua_type_of<basic_table_core<b, Base>> : std::integral_constant<type, type::table> { };

		template <>
		struct lua_type_of<reference> : std::integral_constant<type, type::poly> {};

		template <>
		struct lua_type_of<stack_reference> : std::integral_constant<type, type::poly> {};

		template <typename Base>
		struct lua_type_of<basic_object<Base>> : std::integral_constant<type, type::poly> {};

		template <typename... Args>
		struct lua_type_of<std::tuple<Args...>> : std::integral_constant<type, type::poly> {};

		template <typename A, typename B>
		struct lua_type_of<std::pair<A, B>> : std::integral_constant<type, type::poly> {};

		template <>
		struct lua_type_of<void*> : std::integral_constant<type, type::lightuserdata> {};

		template <>
		struct lua_type_of<lightuserdata_value> : std::integral_constant<type, type::lightuserdata> {};

		template <>
		struct lua_type_of<userdata_value> : std::integral_constant<type, type::userdata> {};

		template <typename T>
		struct lua_type_of<light<T>> : std::integral_constant<type, type::lightuserdata> {};

		template <typename T>
		struct lua_type_of<user<T>> : std::integral_constant<type, type::userdata> {};

		template <typename Base>
		struct lua_type_of<basic_lightuserdata<Base>> : std::integral_constant<type, type::lightuserdata> {};

		template <typename Base>
		struct lua_type_of<basic_userdata<Base>> : std::integral_constant<type, type::userdata> {};

		template <>
		struct lua_type_of<lua_CFunction> : std::integral_constant<type, type::function> {};

		template <>
		struct lua_type_of<std::remove_pointer_t<lua_CFunction>> : std::integral_constant<type, type::function> {};

		template <typename Base>
		struct lua_type_of<basic_function<Base>> : std::integral_constant<type, type::function> {};

		template <typename Base>
		struct lua_type_of<basic_protected_function<Base>> : std::integral_constant<type, type::function> {};

		template <>
		struct lua_type_of<coroutine> : std::integral_constant<type, type::function> {};

		template <>
		struct lua_type_of<thread> : std::integral_constant<type, type::thread> {};

		template <typename Signature>
		struct lua_type_of<std::function<Signature>> : std::integral_constant<type, type::function> {};

		template <typename T>
		struct lua_type_of<optional<T>> : std::integral_constant<type, type::poly> {};

		template <>
		struct lua_type_of<variadic_args> : std::integral_constant<type, type::poly> {};

		template <>
		struct lua_type_of<this_state> : std::integral_constant<type, type::poly> {};

		template <>
		struct lua_type_of<type> : std::integral_constant<type, type::poly> {};

		template <typename T>
		struct lua_type_of<T*> : std::integral_constant<type, type::userdata> {};

		template <typename T>
		struct lua_type_of<T, std::enable_if_t<std::is_arithmetic<T>::value>> : std::integral_constant<type, type::number> {};

		template <typename T>
		struct lua_type_of<T, std::enable_if_t<std::is_enum<T>::value>> : std::integral_constant<type, type::number> {};

		template <typename T, typename C = void>
		struct is_container : std::false_type {};

		template <typename T>
		struct is_container<T, std::enable_if_t<meta::has_begin_end<meta::unqualified_t<T>>::value>> : std::true_type {};

		template <>
		struct lua_type_of<meta_function> : std::integral_constant<type, type::string> {};

		template <typename C, C v, template <typename...> class V, typename... Args>
		struct accumulate : std::integral_constant<C, v> {};

		template <typename C, C v, template <typename...> class V, typename T, typename... Args>
		struct accumulate<C, v, V, T, Args...> : accumulate<C, v + V<T>::value, V, Args...> {};
	} // detail

	template <typename T>
	struct is_unique_usertype : std::integral_constant<bool, unique_usertype_traits<T>::value> {};

	template <typename T>
	struct lua_type_of : detail::lua_type_of<T> {};

	template <typename T>
	struct lua_size : std::integral_constant<int, 1> { };

	template <typename A, typename B>
	struct lua_size<std::pair<A, B>> : std::integral_constant<int, lua_size<A>::value + lua_size<B>::value> { };

	template <typename... Args>
	struct lua_size<std::tuple<Args...>> : std::integral_constant<int, detail::accumulate<int, 0, lua_size, Args...>::value> { };

	template <typename T>
	struct is_lua_primitive : std::integral_constant<bool,
		type::userdata != lua_type_of<meta::unqualified_t<T>>::value
		|| (lua_size<T>::value > 1)
		|| std::is_base_of<reference, meta::unqualified_t<T>>::value
		|| std::is_base_of<stack_reference, meta::unqualified_t<T>>::value
		|| meta::is_specialization_of<std::tuple, meta::unqualified_t<T>>::value
		|| meta::is_specialization_of<std::pair, meta::unqualified_t<T>>::value
	> { };

	template <typename T>
	struct is_lua_reference : std::integral_constant<bool,
		std::is_base_of<reference, meta::unqualified_t<T>>::value
		|| std::is_base_of<stack_reference, meta::unqualified_t<T>>::value
		|| meta::is_specialization_of<proxy, meta::unqualified_t<T>>::value
	> { };

	template <typename T>
	struct is_lua_primitive<T*> : std::true_type {};
	template <typename T>
	struct is_lua_primitive<std::reference_wrapper<T>> : std::true_type { };
	template <typename T>
	struct is_lua_primitive<user<T>> : std::true_type { };
	template <typename T>
	struct is_lua_primitive<light<T>> : is_lua_primitive<T*> { };
	template <typename T>
	struct is_lua_primitive<optional<T>> : std::true_type {};
	template <>
	struct is_lua_primitive<userdata_value> : std::true_type {};
	template <>
	struct is_lua_primitive<lightuserdata_value> : std::true_type {};
	template <typename T>
	struct is_lua_primitive<non_null<T>> : is_lua_primitive<T*> {};

	template <typename T>
	struct is_proxy_primitive : is_lua_primitive<T> { };

	template <typename T>
	struct is_transparent_argument : std::false_type {};

	template <>
	struct is_transparent_argument<this_state> : std::true_type {};

	template <>
	struct is_transparent_argument<variadic_args> : std::true_type {};

	template <typename T>
	struct is_variadic_arguments : std::is_same<T, variadic_args> {};

	template <typename Signature>
	struct lua_bind_traits : meta::bind_traits<Signature> {
	private:
		typedef meta::bind_traits<Signature> base_t;
	public:
		typedef std::integral_constant<bool, meta::count_for<is_transparent_argument, typename base_t::args_list>::value != 0> runtime_variadics_t;
		static const std::size_t true_arity = base_t::arity;
		static const std::size_t arity = base_t::arity - meta::count_for<is_transparent_argument, typename base_t::args_list>::value;
		static const std::size_t true_free_arity = base_t::free_arity;
		static const std::size_t free_arity = base_t::free_arity - meta::count_for<is_transparent_argument, typename base_t::args_list>::value;
	};

	template <typename T>
	struct is_table : std::false_type {};
	template <bool x, typename T>
	struct is_table<basic_table_core<x, T>> : std::true_type {};

	template <typename T>
	struct is_function : std::false_type {};
	template <typename T>
	struct is_function<basic_function<T>> : std::true_type {};
	template <typename T>
	struct is_function<basic_protected_function<T>> : std::true_type {};

	template <typename T>
	struct is_lightuserdata : std::false_type {};
	template <typename T>
	struct is_lightuserdata<basic_lightuserdata<T>> : std::true_type {};

	template <typename T>
	struct is_userdata : std::false_type {};
	template <typename T>
	struct is_userdata<basic_userdata<T>> : std::true_type {};

	template <typename T>
	struct is_container : detail::is_container<T>{};
	
	template<typename T>
	inline type type_of() {
		return lua_type_of<meta::unqualified_t<T>>::value;
	}
} // sol

#endif // SOL_TYPES_HPP
