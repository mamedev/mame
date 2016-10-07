#pragma once

#include <cstddef>
#include <string>

namespace sol {
	namespace string_detail {
		struct string_shim {
			std::size_t s;
			const char* p;

			string_shim(const std::string& r) : string_shim(r.data(), r.size()) {}
			string_shim(const char* p) : string_shim(p, std::char_traits<char>::length(p)) {}
			string_shim(const char* p, std::size_t s) : s(s), p(p) {}

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