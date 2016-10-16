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

#ifndef SOL_DEPRECATE_HPP
#define SOL_DEPRECATE_HPP

#ifndef SOL_DEPRECATED
    #ifdef _MSC_VER
        #define SOL_DEPRECATED __declspec(deprecated)
    #elif __GNUC__
        #define SOL_DEPRECATED __attribute__((deprecated)) 
    #else
        #define SOL_DEPRECATED [[deprecated]]
    #endif // compilers
#endif // SOL_DEPRECATED

namespace sol {
	namespace detail {
		template <typename T>
		struct SOL_DEPRECATED deprecate_type {
			using type = T;
		};
	} // detail
} // sol

#endif // SOL_DEPRECATE_HPP
