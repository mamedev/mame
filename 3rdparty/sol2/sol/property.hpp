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

#ifndef SOL_PROPERTY_HPP
#define SOL_PROPERTY_HPP

namespace sol {

	struct no_prop { };

	template <typename R, typename W>
	struct property_wrapper {
		typedef std::integral_constant<bool, !std::is_void<R>::value> can_read;
		typedef std::integral_constant<bool, !std::is_void<W>::value> can_write;
		typedef std::conditional_t<can_read::value, R, no_prop> Read;
		typedef std::conditional_t<can_write::value, W, no_prop> Write;
		Read read;
		Write write;

		template <typename Rx, typename Wx>
		property_wrapper(Rx&& r, Wx&& w) : read(std::forward<Rx>(r)), write(std::forward<Wx>(w)) {}
	};

	namespace property_detail {
		template <typename R, typename W>
		inline decltype(auto) property(std::true_type, R&& read, W&& write) {
			return property_wrapper<std::decay_t<R>, std::decay_t<W>>(std::forward<R>(read), std::forward<W>(write));
		}
		template <typename W, typename R>
		inline decltype(auto) property(std::false_type, W&& write, R&& read) {
			return property_wrapper<std::decay_t<R>, std::decay_t<W>>(std::forward<R>(read), std::forward<W>(write));
		}
		template <typename R>
		inline decltype(auto) property(std::true_type, R&& read) {
			return property_wrapper<std::decay_t<R>, void>(std::forward<R>(read), no_prop());
		}
		template <typename W>
		inline decltype(auto) property(std::false_type, W&& write) {
			return property_wrapper<void, std::decay_t<W>>(no_prop(), std::forward<W>(write));
		}
	} // property_detail

	template <typename F, typename G>
	inline decltype(auto) property(F&& f, G&& g) {
		typedef lua_bind_traits<meta::unqualified_t<F>> left_traits;
		typedef lua_bind_traits<meta::unqualified_t<G>> right_traits;
		return property_detail::property(meta::boolean<(left_traits::free_arity < right_traits::free_arity)>(), std::forward<F>(f), std::forward<G>(g));
	}

	template <typename F>
	inline decltype(auto) property(F&& f) {
		typedef lua_bind_traits<meta::unqualified_t<F>> left_traits;
		return property_detail::property(meta::boolean<(left_traits::free_arity < 2)>(), std::forward<F>(f));
	}

	template <typename F>
	inline decltype(auto) readonly_property(F&& f) {
		return property_detail::property(std::true_type(), std::forward<F>(f));
	}

	// Allow someone to make a member variable readonly (const)
	template <typename R, typename T>
	inline auto readonly(R T::* v) {
		typedef const R C;
		return static_cast<C T::*>(v);
	}

	template <typename T>
	struct var_wrapper {
		T value;
		template <typename... Args>
		var_wrapper(Args&&... args) : value(std::forward<Args>(args)...) {}
		var_wrapper(const var_wrapper&) = default;
		var_wrapper(var_wrapper&&) = default;
		var_wrapper& operator=(const var_wrapper&) = default;
		var_wrapper& operator=(var_wrapper&&) = default;
	};

	template <typename V>
	inline auto var(V&& v) {
		typedef meta::unqualified_t<V> T;
		return var_wrapper<T>(std::forward<V>(v));
	}

} // sol

#endif // SOL_PROPERTY_HPP
