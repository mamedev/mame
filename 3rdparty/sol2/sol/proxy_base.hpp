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

#ifndef SOL_PROXY_BASE_HPP
#define SOL_PROXY_BASE_HPP

#include "reference.hpp"
#include "tuple.hpp"
#include "stack.hpp"

namespace sol {
	template <typename Super>
	struct proxy_base {
		operator std::string() const {
			const Super& super = *static_cast<const Super*>(static_cast<const void*>(this));
			return super.template get<std::string>();
		}

		template<typename T, meta::enable<meta::neg<meta::is_string_constructible<T>>, is_proxy_primitive<meta::unqualified_t<T>>> = meta::enabler>
		operator T () const {
			const Super& super = *static_cast<const Super*>(static_cast<const void*>(this));
			return super.template get<T>();
		}

		template<typename T, meta::enable<meta::neg<meta::is_string_constructible<T>>, meta::neg<is_proxy_primitive<meta::unqualified_t<T>>>> = meta::enabler>
		operator T& () const {
			const Super& super = *static_cast<const Super*>(static_cast<const void*>(this));
			return super.template get<T&>();
		}
	};
} // sol

#endif // SOL_PROXY_BASE_HPP
