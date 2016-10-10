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

#ifndef SOL_TIE_HPP
#define SOL_TIE_HPP

#include "traits.hpp"

namespace sol {

	namespace detail {
		template <typename T>
		struct is_speshul : std::false_type {};
	}

	template <typename T>
	struct tie_size : std::tuple_size<T> {};

	template <typename T>
	struct is_tieable : std::integral_constant<bool, (::sol::tie_size<T>::value > 0)> {};

	template <typename... Tn>
	struct tie_t : public std::tuple<std::add_lvalue_reference_t<Tn>...> {
	private:
		typedef std::tuple<std::add_lvalue_reference_t<Tn>...> base_t;

		template <typename T>
		void set(std::false_type, T&& target) {
			std::get<0>(*this) = std::forward<T>(target);
		}

		template <typename T>
		void set(std::true_type, T&& target) {
			typedef tie_size<meta::unqualified_t<T>> value_size;
			typedef tie_size<std::tuple<Tn...>> tie_size;
			typedef std::conditional_t<(value_size::value < tie_size::value), value_size, tie_size> indices_size;
			typedef std::make_index_sequence<indices_size::value> indices;
			set_extra(detail::is_speshul<meta::unqualified_t<T>>(), indices(), std::forward<T>(target));
		}

		template <std::size_t... I, typename T>
		void set_extra(std::true_type, std::index_sequence<I...>, T&& target) {
			using std::get;
			(void)detail::swallow{ 0,
				(get<I>(*this) = get<I>(types<Tn...>(), target), 0)...
				, 0 };
		}

		template <std::size_t... I, typename T>
		void set_extra(std::false_type, std::index_sequence<I...>, T&& target) {
			using std::get;
			(void)detail::swallow{ 0,
				(get<I>(*this) = get<I>(target), 0)...
				, 0 };
		}

	public:
		using base_t::base_t;

		template <typename T>
		tie_t& operator= (T&& value) {
			typedef is_tieable<meta::unqualified_t<T>> tieable;
			set(tieable(), std::forward<T>(value));
			return *this;
		}

	};

	template <typename... Tn>
	struct tie_size< tie_t<Tn...> > : std::tuple_size< std::tuple<Tn...> > { };

	namespace adl_barrier_detail {
		template <typename... Tn>
		inline tie_t<std::remove_reference_t<Tn>...> tie(Tn&&... argn) {
			return tie_t<std::remove_reference_t<Tn>...>(std::forward<Tn>(argn)...);
		}
	}

	using namespace adl_barrier_detail;

} // sol

#endif // SOL_TIE_HPP
