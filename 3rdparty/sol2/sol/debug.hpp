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

#ifndef SOL_DEBUG_HPP
#define SOL_DEBUG_HPP

#include "stack.hpp"
#include <iostream>

namespace sol {
	namespace detail {
		namespace debug {
			inline std::string dump_types(lua_State* L) {
				std::string visual;
				std::size_t size = lua_gettop(L) + 1;
				for (std::size_t i = 1; i < size; ++i) {
					if (i != 1) {
						visual += " | ";
					}
					visual += type_name(L, stack::get<type>(L, static_cast<int>(i)));
				}
				return visual;
			}

			inline void print_stack(lua_State* L) {
				std::cout << dump_types(L) << std::endl;
			}

			inline void print_section(const std::string& message, lua_State* L) {
				std::cout << "-- " << message << " -- [ " << dump_types(L) << " ]" << std::endl;
			}
		} // detail
	} // debug
} // sol

#endif // SOL_DEBUG_HPP
