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

#ifndef SOL_RAII_HPP
#define SOL_RAII_HPP

#include <memory>
#include "traits.hpp"

namespace sol {
	namespace detail {
		struct default_construct {
			template<typename T, typename... Args>
			static void construct(T&& obj, Args&&... args) {
				std::allocator<meta::unqualified_t<T>> alloc{};
				alloc.construct(obj, std::forward<Args>(args)...);
			}

			template<typename T, typename... Args>
			void operator()(T&& obj, Args&&... args) const {
				construct(std::forward<T>(obj), std::forward<Args>(args)...);
			}
		};

		struct default_destruct {
			template<typename T>
			static void destroy(T&& obj) {
				std::allocator<meta::unqualified_t<T>> alloc{};
				alloc.destroy(obj);
			}

			template<typename T>
			void operator()(T&& obj) const {
				destroy(std::forward<T>(obj));
			}
		};

		struct deleter {
			template <typename T>
			void operator()(T* p) const {
				delete p;
			}
		};

		template <typename T, typename Dx, typename... Args>
		inline std::unique_ptr<T, Dx> make_unique_deleter(Args&&... args) {
			return std::unique_ptr<T, Dx>(new T(std::forward<Args>(args)...));
		}

		template <typename Tag, typename T>
		struct tagged {
			T value;
			template <typename Arg, typename... Args, meta::disable<std::is_same<meta::unqualified_t<Arg>, tagged>> = meta::enabler>
			tagged(Arg&& arg, Args&&... args) : value(std::forward<Arg>(arg), std::forward<Args>(args)...) {}
		};
	} // detail

	template <typename... Args>
	struct constructor_list {};

	template<typename... Args>
	using constructors = constructor_list<Args...>;

	const auto default_constructor = constructors<types<>>{};

	struct no_construction {};
	const auto no_constructor = no_construction{};

	struct call_construction {};
	const auto call_constructor = call_construction{};

	template <typename... Functions>
	struct constructor_wrapper {
		std::tuple<Functions...> functions;
		template <typename Arg, typename... Args, meta::disable<std::is_same<meta::unqualified_t<Arg>, constructor_wrapper>> = meta::enabler>
		constructor_wrapper(Arg&& arg, Args&&... args) : functions(std::forward<Arg>(arg), std::forward<Args>(args)...) {}
	};

	template <typename... Functions>
	inline auto initializers(Functions&&... functions) {
		return constructor_wrapper<std::decay_t<Functions>...>(std::forward<Functions>(functions)...);
	}

	template <typename... Functions>
	struct factory_wrapper {
		std::tuple<Functions...> functions;
		template <typename Arg, typename... Args, meta::disable<std::is_same<meta::unqualified_t<Arg>, factory_wrapper>> = meta::enabler>
		factory_wrapper(Arg&& arg, Args&&... args) : functions(std::forward<Arg>(arg), std::forward<Args>(args)...) {}
	};

	template <typename... Functions>
	inline auto factories(Functions&&... functions) {
		return factory_wrapper<std::decay_t<Functions>...>(std::forward<Functions>(functions)...);
	}

	template <typename Function>
	struct destructor_wrapper {
		Function fx;
		destructor_wrapper(Function f) : fx(std::move(f)) {}
	};

	template <>
	struct destructor_wrapper<void> {};

	const destructor_wrapper<void> default_destructor{};

	template <typename Fx>
	inline auto destructor(Fx&& fx) {
		return destructor_wrapper<std::decay_t<Fx>>(std::forward<Fx>(fx));
	}

} // sol

#endif // SOL_RAII_HPP
