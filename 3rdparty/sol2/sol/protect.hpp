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

#ifndef SOL_PROTECT_HPP
#define SOL_PROTECT_HPP

#include "traits.hpp"
#include <utility>

namespace sol {
	
	template <typename T>
	struct protect_t {
		T value;

		template <typename Arg, typename... Args, meta::disable<std::is_same<protect_t, meta::unqualified_t<Arg>>> = meta::enabler>
		protect_t(Arg&& arg, Args&&... args) : value(std::forward<Arg>(arg), std::forward<Args>(args)...) {}

		protect_t(const protect_t&) = default;
		protect_t(protect_t&&) = default;
		protect_t& operator=(const protect_t&) = default;
		protect_t& operator=(protect_t&&) = default;

	};

	template <typename T>
	auto protect(T&& value) {
		return protect_t<std::decay_t<T>>(std::forward<T>(value));
	}

} // sol

#endif // SOL_PROTECT_HPP
