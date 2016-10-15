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

#ifndef SOL_ERROR_HPP
#define SOL_ERROR_HPP

#include <stdexcept>
#include <string>

namespace sol {
	namespace detail {
		struct direct_error_tag {};
		const auto direct_error = direct_error_tag{};
	} // detail
	
	class error : public std::runtime_error {
	private:
		// Because VC++ is a fuccboi
		std::string w;
	public:
		error(const std::string& str) : error(detail::direct_error, "lua: error: " + str) {}
		error(detail::direct_error_tag, const std::string& str) : std::runtime_error(""), w(str) {}
		error(detail::direct_error_tag, std::string&& str) : std::runtime_error(""), w(std::move(str)) {}

		error(const error& e) = default;
		error(error&& e) = default;
		error& operator=(const error& e) = default;
		error& operator=(error&& e) = default;

		virtual const char* what() const noexcept override {
			return w.c_str();
		}
	};

} // sol

#endif // SOL_ERROR_HPP
