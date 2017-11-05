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

#ifndef SOL_INHERITANCE_HPP
#define SOL_INHERITANCE_HPP

#include "types.hpp"
#include <atomic>

namespace sol {
	template <typename... Args>
	struct base_list { };
	template <typename... Args>
	using bases = base_list<Args...>;

	typedef bases<> base_classes_tag;
	const auto base_classes = base_classes_tag();

	namespace detail {

		template <typename T>
		struct has_derived {
			static bool value;
		};

		template <typename T>
		bool has_derived<T>::value = false;

		inline std::size_t unique_id() {
			static std::atomic<std::size_t> x(0);
			return ++x;
		}

		template <typename T>
		struct id_for {
			static const std::size_t value;
		};

		template <typename T>
		const std::size_t id_for<T>::value = unique_id();

		inline decltype(auto) base_class_check_key() {
			static const auto& key = "class_check";
			return key;
		}

		inline decltype(auto) base_class_cast_key() {
			static const auto& key = "class_cast";
			return key;
		}

		inline decltype(auto) base_class_index_propogation_key() {
			static const auto& key = u8"\xF0\x9F\x8C\xB2.index";
			return key;
		}

		inline decltype(auto) base_class_new_index_propogation_key() {
			static const auto& key = u8"\xF0\x9F\x8C\xB2.new_index";
			return key;
		}

		template <typename T, typename... Bases>
		struct inheritance {
			static bool type_check_bases(types<>, std::size_t) {
				return false;
			}

			template <typename Base, typename... Args>
			static bool type_check_bases(types<Base, Args...>, std::size_t ti) {
				return ti == id_for<Base>::value || type_check_bases(types<Args...>(), ti);
			}

			static bool type_check(std::size_t ti) {
				return ti == id_for<T>::value || type_check_bases(types<Bases...>(), ti);
			}

			static void* type_cast_bases(types<>, T*, std::size_t) {
				return nullptr;
			}

			template <typename Base, typename... Args>
			static void* type_cast_bases(types<Base, Args...>, T* data, std::size_t ti) {
				// Make sure to convert to T first, and then dynamic cast to the proper type
				return ti != id_for<Base>::value ? type_cast_bases(types<Args...>(), data, ti) : static_cast<void*>(static_cast<Base*>(data));
			}

			static void* type_cast(void* voiddata, std::size_t ti) {
				T* data = static_cast<T*>(voiddata);
				return static_cast<void*>(ti != id_for<T>::value ? type_cast_bases(types<Bases...>(), data, ti) : data);
			}
		};

		using inheritance_check_function = decltype(&inheritance<void>::type_check);
		using inheritance_cast_function = decltype(&inheritance<void>::type_cast);

	} // detail
} // sol

#endif // SOL_INHERITANCE_HPP
