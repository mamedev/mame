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

#ifndef SOL_TRAITS_HPP
#define SOL_TRAITS_HPP

#include "tuple.hpp"
#include "bind_traits.hpp"
#include <type_traits>
#include <memory>
#include <functional>

namespace sol {
	template<std::size_t I>
	using index_value = std::integral_constant<std::size_t, I>;

	namespace meta {
		template<typename T>
		struct identity { typedef T type; };

		template<typename T>
		using identity_t = typename identity<T>::type;

		template<typename... Args>
		struct is_tuple : std::false_type { };

		template<typename... Args>
		struct is_tuple<std::tuple<Args...>> : std::true_type { };

		template <typename T>
		struct is_builtin_type : std::integral_constant<bool, std::is_arithmetic<T>::value || std::is_pointer<T>::value || std::is_array<T>::value> {};

		template<typename T>
		struct unwrapped {
			typedef T type;
		};

		template<typename T>
		struct unwrapped<std::reference_wrapper<T>> {
			typedef T type;
		};

		template<typename T>
		using unwrapped_t = typename unwrapped<T>::type;

		template <typename T>
		struct unwrap_unqualified : unwrapped<unqualified_t<T>> {};

		template <typename T>
		using unwrap_unqualified_t = typename unwrap_unqualified<T>::type;

		template<typename T>
		struct remove_member_pointer;

		template<typename R, typename T>
		struct remove_member_pointer<R T::*> {
			typedef R type;
		};

		template<typename R, typename T>
		struct remove_member_pointer<R T::* const> {
			typedef R type;
		};

		template<typename T>
		using remove_member_pointer_t = remove_member_pointer<T>;

		template<template<typename...> class Templ, typename T>
		struct is_specialization_of : std::false_type { };
		template<typename... T, template<typename...> class Templ>
		struct is_specialization_of<Templ, Templ<T...>> : std::true_type { };

		template<class T, class...>
		struct all_same : std::true_type { };

		template<class T, class U, class... Args>
		struct all_same<T, U, Args...> : std::integral_constant <bool, std::is_same<T, U>::value && all_same<T, Args...>::value> { };

		template<class T, class...>
		struct any_same : std::false_type { };

		template<class T, class U, class... Args>
		struct any_same<T, U, Args...> : std::integral_constant <bool, std::is_same<T, U>::value || any_same<T, Args...>::value> { };

		template<typename T>
		using invoke_t = typename T::type;

		template<bool B>
		using boolean = std::integral_constant<bool, B>;

		template<typename T>
		using neg = boolean<!T::value>;

		template<typename Condition, typename Then, typename Else>
		using condition = std::conditional_t<Condition::value, Then, Else>;

		template<typename... Args>
		struct all : boolean<true> {};

		template<typename T, typename... Args>
		struct all<T, Args...> : condition<T, all<Args...>, boolean<false>> {};

		template<typename... Args>
		struct any : boolean<false> {};

		template<typename T, typename... Args>
		struct any<T, Args...> : condition<T, boolean<true>, any<Args...>> {};

		enum class enable_t {
			_
		};

		constexpr const auto enabler = enable_t::_;

		template<bool value, typename T = void>
		using disable_if_t = std::enable_if_t<!value, T>;

		template<typename... Args>
		using enable = std::enable_if_t<all<Args...>::value, enable_t>;

		template<typename... Args>
		using disable = std::enable_if_t<neg<all<Args...>>::value, enable_t>;

		template<typename... Args>
		using disable_any = std::enable_if_t<neg<any<Args...>>::value, enable_t>;

		template<typename V, typename... Vs>
		struct find_in_pack_v : boolean<false> { };

		template<typename V, typename Vs1, typename... Vs>
		struct find_in_pack_v<V, Vs1, Vs...> : any<boolean<(V::value == Vs1::value)>, find_in_pack_v<V, Vs...>> { };

		namespace meta_detail {
			template<std::size_t I, typename T, typename... Args>
			struct index_in_pack : std::integral_constant<std::size_t, SIZE_MAX> { };

			template<std::size_t I, typename T, typename T1, typename... Args>
			struct index_in_pack<I, T, T1, Args...> : std::conditional_t<std::is_same<T, T1>::value, std::integral_constant<std::ptrdiff_t, I>, index_in_pack<I + 1, T, Args...>> { };
		}

		template<typename T, typename... Args>
		struct index_in_pack : meta_detail::index_in_pack<0, T, Args...> { };

		template<typename T, typename List>
		struct index_in : meta_detail::index_in_pack<0, T, List> { };

		template<typename T, typename... Args>
		struct index_in<T, types<Args...>> : meta_detail::index_in_pack<0, T, Args...> { };

		template<std::size_t I, typename... Args>
		struct at_in_pack {};

		template<std::size_t I, typename... Args>
		using at_in_pack_t = typename at_in_pack<I, Args...>::type;

		template<std::size_t I, typename Arg, typename... Args>
		struct at_in_pack<I, Arg, Args...> : std::conditional<I == 0, Arg, at_in_pack_t<I - 1, Args...>> {};

		template<typename Arg, typename... Args>
		struct at_in_pack<0, Arg, Args...> { typedef Arg type; };

		namespace meta_detail {
			template<std::size_t Limit, std::size_t I, template<typename...> class Pred, typename... Ts>
			struct count_for_pack : std::integral_constant<std::size_t, 0> {};
			template<std::size_t Limit, std::size_t I, template<typename...> class Pred, typename T, typename... Ts>
			struct count_for_pack<Limit, I, Pred, T, Ts...> : std::conditional_t < sizeof...(Ts) == 0 || Limit < 2,
				std::integral_constant<std::size_t, I + static_cast<std::size_t>(Limit != 0 && Pred<T>::value)>,
				count_for_pack<Limit - 1, I + static_cast<std::size_t>(Pred<T>::value), Pred, Ts...>
			> { };
			template<std::size_t I, template<typename...> class Pred, typename... Ts>
			struct count_2_for_pack : std::integral_constant<std::size_t, 0> {};
			template<std::size_t I, template<typename...> class Pred, typename T, typename U, typename... Ts>
			struct count_2_for_pack<I, Pred, T, U, Ts...> : std::conditional_t<sizeof...(Ts) == 0,
				std::integral_constant<std::size_t, I + static_cast<std::size_t>(Pred<T>::value)>,
				count_2_for_pack<I + static_cast<std::size_t>(Pred<T>::value), Pred, Ts...>
			> { };
		} // meta_detail

		template<template<typename...> class Pred, typename... Ts>
		struct count_for_pack : meta_detail::count_for_pack<sizeof...(Ts), 0, Pred, Ts...> { };

		template<template<typename...> class Pred, typename List>
		struct count_for;

		template<template<typename...> class Pred, typename... Args>
		struct count_for<Pred, types<Args...>> : count_for_pack<Pred, Args...> {};

		template<std::size_t Limit, template<typename...> class Pred, typename... Ts>
		struct count_for_to_pack : meta_detail::count_for_pack<Limit, 0, Pred, Ts...> { };

		template<template<typename...> class Pred, typename... Ts>
		struct count_2_for_pack : meta_detail::count_2_for_pack<0, Pred, Ts...> { };

		template<typename... Args>
		struct return_type {
			typedef std::tuple<Args...> type;
		};

		template<typename T>
		struct return_type<T> {
			typedef T type;
		};

		template<>
		struct return_type<> {
			typedef void type;
		};

		template <typename... Args>
		using return_type_t = typename return_type<Args...>::type;

		namespace meta_detail {
			template <typename> struct always_true : std::true_type {};
			struct is_invokable_tester {
				template <typename Fun, typename... Args>
				always_true<decltype(std::declval<Fun>()(std::declval<Args>()...))> static test(int);
				template <typename...>
				std::false_type static test(...);
			};
		} // meta_detail

		template <typename T>
		struct is_invokable;
		template <typename Fun, typename... Args>
		struct is_invokable<Fun(Args...)> : decltype(meta_detail::is_invokable_tester::test<Fun, Args...>(0)) {};

		namespace meta_detail {

			template<typename T, bool isclass = std::is_class<unqualified_t<T>>::value>
			struct is_callable : std::is_function<std::remove_pointer_t<T>> {};

			template<typename T>
			struct is_callable<T, true> {
				using yes = char;
				using no = struct { char s[2]; };

				struct F { void operator()() {}; };
				struct Derived : T, F {};
				template<typename U, U> struct Check;

				template<typename V>
				static no test(Check<void (F::*)(), &V::operator()>*);

				template<typename>
				static yes test(...);

				static const bool value = sizeof(test<Derived>(0)) == sizeof(yes);
			};

			struct has_begin_end_impl {
				template<typename T, typename U = unqualified_t<T>,
					typename B = decltype(std::declval<U&>().begin()),
					typename E = decltype(std::declval<U&>().end())>
					static std::true_type test(int);

				template<typename...>
				static std::false_type test(...);
			};

			struct has_key_value_pair_impl {
				template<typename T, typename U = unqualified_t<T>,
					typename V = typename U::value_type,
					typename F = decltype(std::declval<V&>().first),
					typename S = decltype(std::declval<V&>().second)>
					static std::true_type test(int);

				template<typename...>
				static std::false_type test(...);
			};

			template <typename T, typename U = T, typename = decltype(std::declval<T&>() < std::declval<U&>())>
			std::true_type supports_op_less_test(const T&);
			std::false_type supports_op_less_test(...);
			template <typename T, typename U = T, typename = decltype(std::declval<T&>() == std::declval<U&>())>
			std::true_type supports_op_equal_test(const T&);
			std::false_type supports_op_equal_test(...);
			template <typename T, typename U = T, typename = decltype(std::declval<T&>() <= std::declval<U&>())>
			std::true_type supports_op_less_equal_test(const T&);
			std::false_type supports_op_less_equal_test(...);

		} // meta_detail

		template <typename T>
		using supports_op_less = decltype(meta_detail::supports_op_less_test(std::declval<T&>()));
		template <typename T>
		using supports_op_equal = decltype(meta_detail::supports_op_equal_test(std::declval<T&>()));
		template <typename T>
		using supports_op_less_equal = decltype(meta_detail::supports_op_less_equal_test(std::declval<T&>()));

		template<typename T>
		struct is_callable : boolean<meta_detail::is_callable<T>::value> {};

		template<typename T>
		struct has_begin_end : decltype(meta_detail::has_begin_end_impl::test<T>(0)) {};

		template<typename T>
		struct has_key_value_pair : decltype(meta_detail::has_key_value_pair_impl::test<T>(0)) {};

		template <typename T>
		using is_string_constructible = any<std::is_same<unqualified_t<T>, const char*>, std::is_same<unqualified_t<T>, char>, std::is_same<unqualified_t<T>, std::string>, std::is_same<unqualified_t<T>, std::initializer_list<char>>>;

		template <typename T>
		using is_c_str = any<
			std::is_same<std::decay_t<unqualified_t<T>>, const char*>,
			std::is_same<std::decay_t<unqualified_t<T>>, char*>,
			std::is_same<unqualified_t<T>, std::string>
		>;

		template <typename T>
		struct is_move_only : all<
			neg<std::is_reference<T>>,
			neg<std::is_copy_constructible<unqualified_t<T>>>,
			std::is_move_constructible<unqualified_t<T>>
		> {};

		template <typename T>
		using is_not_move_only = neg<is_move_only<T>>;

		namespace meta_detail {
			template <typename T, meta::disable<meta::is_specialization_of<std::tuple, meta::unqualified_t<T>>> = meta::enabler>
			decltype(auto) force_tuple(T&& x) {
				return std::forward_as_tuple(std::forward<T>(x));
			}

			template <typename T, meta::enable<meta::is_specialization_of<std::tuple, meta::unqualified_t<T>>> = meta::enabler>
			decltype(auto) force_tuple(T&& x) {
				return std::forward<T>(x);
			}
		} // meta_detail

		template <typename... X>
		decltype(auto) tuplefy(X&&... x) {
			return std::tuple_cat(meta_detail::force_tuple(std::forward<X>(x))...);
		}
	} // meta
	namespace detail {
		template <std::size_t I, typename Tuple>
		decltype(auto) forward_get(Tuple&& tuple) {
			return std::forward<meta::tuple_element_t<I, Tuple>>(std::get<I>(tuple));
		}

		template <std::size_t... I, typename Tuple>
		auto forward_tuple_impl(std::index_sequence<I...>, Tuple&& tuple) -> decltype(std::tuple<decltype(forward_get<I>(tuple))...>(forward_get<I>(tuple)...)) {
			return std::tuple<decltype(forward_get<I>(tuple))...>(std::move(std::get<I>(tuple))...);
		}

		template <typename Tuple>
		auto forward_tuple(Tuple&& tuple) {
			auto x = forward_tuple_impl(std::make_index_sequence<std::tuple_size<meta::unqualified_t<Tuple>>::value>(), std::forward<Tuple>(tuple));
			return x;
		}

		template<typename T>
		auto unwrap(T&& item) -> decltype(std::forward<T>(item)) {
			return std::forward<T>(item);
		}

		template<typename T>
		T& unwrap(std::reference_wrapper<T> arg) {
			return arg.get();
		}

		template<typename T>
		auto deref(T&& item) -> decltype(std::forward<T>(item)) {
			return std::forward<T>(item);
		}

		template<typename T>
		inline T& deref(T* item) {
			return *item;
		}

		template<typename T, typename Dx>
		inline std::add_lvalue_reference_t<T> deref(std::unique_ptr<T, Dx>& item) {
			return *item;
		}

		template<typename T>
		inline std::add_lvalue_reference_t<T> deref(std::shared_ptr<T>& item) {
			return *item;
		}

		template<typename T, typename Dx>
		inline std::add_lvalue_reference_t<T> deref(const std::unique_ptr<T, Dx>& item) {
			return *item;
		}

		template<typename T>
		inline std::add_lvalue_reference_t<T> deref(const std::shared_ptr<T>& item) {
			return *item;
		}

		template<typename T>
		inline T* ptr(T& val) {
			return std::addressof(val);
		}

		template<typename T>
		inline T* ptr(std::reference_wrapper<T> val) {
			return std::addressof(val.get());
		}

		template<typename T>
		inline T* ptr(T* val) {
			return val;
		}
	} // detail
} // sol

#endif // SOL_TRAITS_HPP
