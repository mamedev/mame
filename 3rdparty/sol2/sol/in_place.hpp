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

#ifndef SOL_IN_PLACE_HPP
#define SOL_IN_PLACE_HPP

namespace sol {

	namespace detail {
		struct in_place_of {};
		template <std::size_t I>
		struct in_place_of_i {};
		template <typename T>
		struct in_place_of_t {};
	} // detail

	struct in_place_tag { struct init {}; constexpr in_place_tag(init) {} in_place_tag() = delete; };
	constexpr inline in_place_tag in_place(detail::in_place_of) { return in_place_tag(in_place_tag::init()); }
	template <typename T>
	constexpr inline in_place_tag in_place(detail::in_place_of_t<T>) { return in_place_tag(in_place_tag::init()); }
	template <std::size_t I>
	constexpr inline in_place_tag in_place(detail::in_place_of_i<I>) { return in_place_tag(in_place_tag::init()); }

	using in_place_t = in_place_tag(&)(detail::in_place_of);
	template <typename T>
	using in_place_type_t = in_place_tag(&)(detail::in_place_of_t<T>);
	template <std::size_t I>
	using in_place_index_t = in_place_tag(&)(detail::in_place_of_i<I>);

} // sol

#endif // SOL_IN_PLACE_HPP
