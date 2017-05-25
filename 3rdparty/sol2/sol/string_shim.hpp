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

#ifndef SOL_STRING_SHIM_HPP
#define SOL_STRING_SHIM_HPP

#include <cstddef>
#include <string>

namespace sol {
	namespace string_detail {
		struct string_shim {
			std::size_t s;
			const char* p;

			string_shim(const std::string& r) : string_shim(r.data(), r.size()) {}
			string_shim(const char* ptr) : string_shim(ptr, std::char_traits<char>::length(ptr)) {}
			string_shim(const char* ptr, std::size_t sz) : s(sz), p(ptr) {}

			static int compare(const char* lhs_p, std::size_t lhs_sz, const char* rhs_p, std::size_t rhs_sz) {
				int result = std::char_traits<char>::compare(lhs_p, rhs_p, lhs_sz < rhs_sz ? lhs_sz : rhs_sz);
				if (result != 0)
					return result;
				if (lhs_sz < rhs_sz)
					return -1;
				if (lhs_sz > rhs_sz)
					return 1;
				return 0;
			}

			const char* c_str() const {
				return p;
			}

			const char* data() const {
				return p;
			}

			std::size_t size() const {
				return s;
			}

			bool operator==(const string_shim& r) const {
				return compare(p, s, r.data(), r.size()) == 0;
			}

			bool operator==(const char* r) const {
				return compare(r, std::char_traits<char>::length(r), p, s) == 0;
			}

			bool operator==(const std::string& r) const {
				return compare(r.data(), r.size(), p, s) == 0;
			}

			bool operator!=(const string_shim& r) const {
				return !(*this == r);
			}

			bool operator!=(const char* r) const {
				return !(*this == r);
			}

			bool operator!=(const std::string& r) const {
				return !(*this == r);
			}
		};
	}
}

#endif // SOL_STRING_SHIM_HPP
