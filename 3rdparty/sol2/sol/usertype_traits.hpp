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

#ifndef SOL_USERTYPE_TRAITS_HPP
#define SOL_USERTYPE_TRAITS_HPP

#include "demangle.hpp"

namespace sol {

	template<typename T>
	struct usertype_traits {
		static const std::string name;
		static const std::string qualified_name;
		static const std::string metatable;
		static const std::string user_gc_metatable;
		static const std::string gc_table;
	};

	template<typename T>
	const std::string usertype_traits<T>::name = detail::short_demangle<T>();

	template<typename T>
	const std::string usertype_traits<T>::qualified_name = detail::demangle<T>();

	template<typename T>
	const std::string usertype_traits<T>::metatable = std::string("sol.").append(detail::demangle<T>());

	template<typename T>
	const std::string usertype_traits<T>::user_gc_metatable = std::string("sol.").append(detail::demangle<T>()).append(".user\xE2\x99\xBB");

	template<typename T>
	const std::string usertype_traits<T>::gc_table = std::string("sol.").append(detail::demangle<T>().append(".\xE2\x99\xBB"));

}

#endif // SOL_USERTYPE_TRAITS_HPP
